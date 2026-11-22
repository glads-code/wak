# wak

awk des scripts du poste, sous Linux comme sous Windows : [wak](https://github.com/raygard/wak) de Ray Gardner (awk de toybox, licence 0BSD, voir `LICENSE`), en version un seul fichier (`wak.c`), avec un correctif.

## Pourquoi wak

- Même awk des deux côtés (mawk n'a pas de binaire Windows officiel).
- UTF-8 : `length`, `substr`, `index`, `toupper`, `tolower` et les classes `[…]` travaillent en caractères, sans dépendre de la locale.
- Petit (un fichier C, binaire d'environ 90 Ko), se compile avec tcc.

À savoir :
- Pas de `strftime` ni de `systime` : la date vient de `date` (9base, 9pm), lue par wak.
- Les nombres s’écrivent toujours avec un point, quelle que soit la locale (voir Correctifs).
- L'ordre de `for (k in t)` n'est pas celui de mawk : un résultat ne doit jamais en dépendre (trier les clés).
- Environ 1,5 fois plus lent que mawk sur les gros fichiers de dvf.

## Correctifs

Le reste de `wak.c` est la source d'origine (commit `c757c495efdc` de raygard/wak).
- **Doublon dans l'environnement** : wak s'arrêtait (« env var dup? ») quand une variable apparaît deux fois, ce que fait le rc de 9base quand un script rc en lance un autre qui redéfinit la même fonction (`fn#tube`). Il garde maintenant le premier exemplaire, comme `getenv()`.
- **Locale numérique** : wak ne charge plus `LC_NUMERIC`. En locale française, il écrivait `3,5` et `printf` se trompait dès deux conversions (`"%d %d"` : « too few args for printf »). Les nombres sont maintenant toujours écrits et lus avec un point.

## Compiler et installer

Linux (deb) : `./compiler.rc` compile avec tcc, lance `./essai.rc` sur le binaire et l'installe dans `/usr/local/bin/wak` (dans le PATH des tâches systemd).

`./essai.rc <binaire>` vérifie ce dont nos scripts dépendent : UTF-8, nombres (en locale française aussi), `nextfile`, doublon d'environnement.

## Windows

`./compiler-windows.rc` produit `wak.exe` (64 bits) sur deb, avec le tcc croisé x86_64-win32 compilé depuis les sources de tinycc (`https://repo.or.cz/tinycc.git`, dans `~/.cache/wak-src/tinycc` par défaut, autre dossier en argument) :

    ./configure --targetos=WIN32 --cpu=x86_64 --enable-cross
    make cross-x86_64-win32

Vérifier ensuite sous Windows, avec le rcsh corrigé (dépôt `rcsh`) : `rcsh essai.rc wak.exe < NUL`.

### Regex

wak utilise `regex.h` (POSIX), absent de Windows. Le dossier `regex/` reprend la regex de musl 1.2.5 (licence MIT, voir `regex/COPYRIGHT`), adaptée :
- elle se compile hors de musl (`regex.h` sans en-têtes internes, `regerror` sans traduction, limites `CHARCLASS_NAME_MAX` et `RE_DUP_MAX` définies) ;
- elle lit l'UTF-8 quelle que soit la locale (décodeur en tête de `tre.h`), car le msvcrt de Windows n'a pas de locale UTF-8 ;
- les classes `[:alpha:]`… passent par les fonctions `isw…` (le msvcrt n'a pas `wctype()`) ;
- hors du plan de base Unicode (émojis), un caractère devient U+FFFD sous Windows (`wchar_t` sur 16 bits).

Elle n'a pas `REG_STARTEND` : `regexec0` copie alors la chaîne avant une recherche bornée. Sous Linux, elle se vérifie avec :

    tcc -std=c99 -D_POSIX_C_SOURCE=200809L -Iregex wak.c regex/*.c -o wak-musl -lm
    ./essai.rc ./wak-musl

Elle donne les mêmes sorties que la regex de la glibc, environ 1,7 fois plus lentement.

### Ce que wak complète au msvcrt

Dans `wak.c`, sous `_WIN32`, et dans `windows/` (`getopt_long`, `getline`) :
- fichiers et tubes en binaire : pas de `\r\n` ajouté ni retiré ;
- `%d`, `%x`… de `printf` en `long long` (`long` n'a que 32 bits sous Windows) ;
- NaN et infini écrits comme la glibc (`-nan`, `inf`), `%c` encodé en UTF-8 par wak.

Écart connu : la glibc lit l'hexadécimal (`"0x10" + 0` vaut 16, et la constante `0x10` aussi), le msvcrt non (0). Aucun de nos programmes ne s'en sert.

Dans la VM Windows 11 : `essai.rc` réussi, et sorties identiques à Linux à l'octet près sur des programmes d'essai (accents, classes, `gsub`, `match`, `split`, `RS` en regex, `nextfile`, nombres et `printf`) et 20 000 lignes de DVF.
