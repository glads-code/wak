# wak

awk portable, le même sous Linux comme sous Windows : [wak](https://github.com/raygard/wak) de Ray Gardner (awk de toybox, licence 0BSD, voir `LICENSE`), en version un seul fichier (`wak.c`), avec un correctif.

## Pourquoi wak

- Même awk des deux côtés (mawk n'a pas de binaire Windows officiel).
- UTF-8 : `length`, `substr`, `index`, `toupper`, `tolower` et les classes `[…]` travaillent en caractères, sans dépendre de la locale.
- Petit (un fichier C, binaire d'environ 90 Ko), se compile avec tcc.

À savoir :
- Pas de `strftime` ni de `systime` : la date vient de `date` (9base, 9pm), lue par wak.
- Les nombres s’écrivent toujours avec un point, quelle que soit la locale (voir Correctifs).
- L'ordre de `for (k in t)` n'est pas celui de mawk : un résultat ne doit jamais en dépendre (trier les clés).
- Environ 1,5 fois plus lent que mawk sur de gros fichiers CSV.

## Correctifs

Le reste de `wak.c` est la source d'origine (commit `c757c495efdc` de raygard/wak).
- **Doublon dans l'environnement** : wak s'arrêtait (« env var dup? ») quand une variable apparaît deux fois, ce que fait le rc de 9base quand un script rc en lance un autre qui redéfinit la même fonction (`fn#tube`). Il garde maintenant le premier exemplaire, comme `getenv()`.
- **Locale numérique** : wak ne charge plus `LC_NUMERIC`. En locale française, il écrivait `3,5` et `printf` se trompait dès deux conversions (`"%d %d"` : « too few args for printf »). Les nombres sont maintenant toujours écrits et lus avec un point.

## Paquets Debian et Fedora

Les [Releases](https://github.com/glads-code/wak/releases) contiennent un `.deb` (Debian 13) et un `.rpm` (Fedora 43), compilés avec gcc :

    sudo apt install ./wak_1.0-1_amd64.deb             # Debian
    sudo dnf install ./wak-1.0-1.fc43.x86_64.rpm       # Fedora

Pour les reconstruire : `./paquets.rc [version]` (Docker, conteneurs `debian:13` et `fedora:43` jetables ; résultat dans `dist/`). Le `.deb` passe `essai.rc` ; Fedora n'ayant pas le rc de Plan 9, le `.rpm` vérifie les mêmes points en sh (`%check` de `paquets/wak.spec`).

## Installer sous Debian 13

    sudo apt install git tcc 9base
    git clone https://github.com/glads-code/wak.git
    cd wak
    ./compiler.rc

`./compiler.rc` compile avec tcc, lance `./essai.rc` sur le binaire et l'installe dans `/usr/local/bin/wak` (dans le PATH des tâches systemd). Vérifier : `echo 'é' | wak '{print length}'` affiche `1`.

### Essai

`./essai.rc <binaire>` vérifie un exécutable de wak. `<binaire>` est le chemin du wak à essayer, par exemple `./essai.rc /usr/local/bin/wak` (le wak installé) ou `./essai.rc ./wak.nouveau` (celui qui vient d'être compilé, avant de l'installer). Il affiche `essai réussi`, ou le premier point en échec avec le résultat obtenu et celui attendu. Points vérifiés :

- **UTF-8** : `length`, `substr` et `toupper` comptent et convertissent des caractères, pas des octets (`église` fait 6 caractères).
- **Nombres** : `7 / 2` s'écrit `3.5`, avec un point, même en locale française (`LC_ALL=fr_FR.UTF-8`), et `printf` à plusieurs conversions fonctionne.
- **`nextfile`** : passe directement au fichier suivant sans lire la suite du fichier en cours (instruction de gawk et de POSIX 2024, absente de certains awk).
- **Variable en double dans l'environnement** : quand un script rc définit une fonction `f`, puis lance un autre rc qui la redéfinit, ce dernier passe aux programmes qu'il lance deux variables `fn#f`. Le wak d'origine s'arrêtait alors (« env var dup? ») ; celui-ci garde la première, comme `getenv()`.

## Installer sous Windows

Binaire prêt à l'emploi : `wak.exe` dans les [Releases](https://github.com/glads-code/wak/releases) (compilé sous Windows 11 selon la procédure ci-dessous, empreinte SHA-256 dans les notes de version). Le copier dans un dossier du `PATH` suffit. Pour le compiler soi-même :

Prérequis : tcc récent dans `C:\tcc-dev`, voir [Installer tcc sous Windows](https://github.com/glads-code/rcsh#installer-tcc-sous-windows) (README de rcsh) ; pour l'essai, 9pm et le rcsh corrigé (même README). Dans PowerShell :

```powershell
cd $env:USERPROFILE
curl.exe -Lo wak.zip https://github.com/glads-code/wak/archive/refs/heads/main.zip
tar -xf wak.zip
cd wak-main
C:\tcc-dev\tcc.exe -std=c99 -w -Iwindows -Iregex -o wak.exe wak.c (Get-ChildItem regex\*.c).FullName windows\compat.c
cmd /c "rcsh essai.rc wak.exe < NUL"    # wak.exe : essai réussi
Copy-Item wak.exe C:\9pm\bin\           # ou tout autre dossier du PATH
```

`wak.exe` : 64 bits, sans DLL.

## Détails Windows

### Regex

wak utilise `regex.h` (POSIX), absent de Windows. Le dossier `regex/` reprend la regex de musl 1.2.5 (licence MIT, voir `regex/COPYRIGHT`), adaptée :
- elle se compile hors de musl (`regex.h` sans en-têtes internes, `regerror` sans traduction, limites `CHARCLASS_NAME_MAX` et `RE_DUP_MAX` définies) ;
- elle lit l'UTF-8 quelle que soit la locale (décodeur en tête de `tre.h`), car le msvcrt de Windows n'a pas de locale UTF-8 ; un octet hors UTF-8 (fichier en Latin-1) y devient un caractère à part, et la recherche continue, comme avec la glibc ;
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

Sous Windows 11 : `essai.rc` réussi, et sorties identiques à Linux à l'octet près sur des programmes d'essai (accents, classes, `gsub`, `match`, `split`, `RS` en regex, `nextfile`, nombres et `printf`) et 20 000 lignes de CSV réel.

## Support

Dépôt publié tel quel, pour nos propres besoins : pas de support, les issues et pull requests ne sont pas suivies.
