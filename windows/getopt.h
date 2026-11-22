/* getopt_long minimal pour wak sous Windows (le msvcrt n'en a pas).
   S'arrête au premier argument qui n'est pas une option, comme POSIX. */
#ifndef WAK_GETOPT_H
#define WAK_GETOPT_H

struct option { const char *name; int has_arg; int *flag; int val; };

static char *optarg;
static int optind = 1, opterr = 1, optopt;

static int getopt_long(int argc, char *const argv[], const char *opts,
  const struct option *longopts, int *idx)
{
  static int pos = 1;
  char *a, *o;
  (void)idx;
  optarg = 0;
  if (optind >= argc) return -1;
  a = argv[optind];
  if (pos == 1) {
    if (a[0] != '-' || !a[1]) return -1;
    if (a[1] == '-' && !a[2]) { optind++; return -1; }
    if (a[1] == '-') {
      const struct option *l;
      optind++;
      for (l = longopts; l && l->name; l++)
        if (!strcmp(a + 2, l->name)) return l->val;
      if (opterr) fprintf(stderr, "option inconnue : %s\n", a);
      return '?';
    }
  }
  optopt = a[pos];
  o = strchr(opts, optopt);
  if (!o || optopt == ':') {
    if (opterr) fprintf(stderr, "option inconnue : -%c\n", optopt);
    if (!a[++pos]) { optind++; pos = 1; }
    return '?';
  }
  if (o[1] == ':') {
    if (a[pos + 1]) optarg = a + pos + 1;
    else if (optind + 1 < argc) optarg = argv[++optind];
    else {
      if (opterr) fprintf(stderr, "argument manquant pour -%c\n", optopt);
      optind++; pos = 1;
      return '?';
    }
    optind++; pos = 1;
    return optopt;
  }
  if (!a[++pos]) { optind++; pos = 1; }
  return optopt;
}
#endif
