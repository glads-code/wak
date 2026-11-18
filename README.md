# wak

awk des scripts du poste, sous Linux comme sous Windows : [wak](https://github.com/raygard/wak) de Ray Gardner (awk de toybox, licence 0BSD, voir `LICENSE`), en version un seul fichier (`wak.c`), avec un correctif.

## Pourquoi wak

- Même awk des deux côtés (mawk n'a pas de binaire Windows officiel).
- UTF-8 : `length`, `substr`, `index`, `toupper`, `tolower` et les classes `[…]` travaillent en caractères, sans dépendre de la locale.
- Petit (un fichier C, binaire d'environ 90 Ko), se compile avec tcc.

À savoir :
- Pas de `strftime` ni de `systime` : la date vient de `date` (9base, 9pm), lue par wak.
- Les nombres suivent la locale (`3,5` en français) : les scripts posent `LC_ALL=C.UTF-8`.
- L'ordre de `for (k in t)` n'est pas celui de mawk : un résultat ne doit jamais en dépendre (trier les clés).
- Environ 1,5 fois plus lent que mawk sur les gros fichiers de dvf.

## Correctif

wak s'arrêtait (« env var dup? ») quand une variable apparaît deux fois dans l'environnement, ce que fait le rc de 9base quand un script rc en lance un autre qui redéfinit la même fonction (`fn#tube`). Il garde maintenant le premier exemplaire, comme `getenv()`. Le reste de `wak.c` est la source d'origine (commit `c757c495efdc` de raygard/wak).

## Compiler et installer

Linux (deb) : `./compiler.rc` compile avec tcc, lance `./essai.rc` sur le binaire et l'installe dans `~/.local/bin/wak`.

`./essai.rc <binaire>` vérifie ce dont nos scripts dépendent : UTF-8, nombres, `nextfile`, doublon d'environnement.

Windows : wak utilise `regex.h` (POSIX), absent de Windows. La compilation d'origine passe par msys2 et sa bibliothèque `libregex` ; une solution avec tcc pour Windows reste à trouver (voir `~/.claude/windows-natif.md`).
