/* Ce qui manque au msvcrt pour wak sous Windows. */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

ssize_t getline(char **lineptr, size_t *n, FILE *f)
{
  size_t len = 0;
  int c;
  if (!*lineptr || !*n) {
    *n = 128;
    if (!(*lineptr = malloc(*n))) return -1;
  }
  while ((c = getc(f)) != EOF) {
    if (len + 2 > *n) {
      char *p = realloc(*lineptr, *n * 2);
      if (!p) return -1;
      *lineptr = p;
      *n *= 2;
    }
    (*lineptr)[len++] = c;
    if (c == '\n') break;
  }
  if (!len) return -1;
  (*lineptr)[len] = 0;
  return len;
}

#include <fcntl.h>
#include <io.h>

/* Lecture et écriture en binaire, comme sous Linux : le mode texte du
   msvcrt changerait chaque \n en \r\n. */
void wak_windows(void)
{
  _fmode = _O_BINARY;
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
  _setmode(_fileno(stderr), _O_BINARY);
}
