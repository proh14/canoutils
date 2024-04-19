#include "./getopt.h"
#include <stdio.h>
#include <stdlib.h>

char *optarg = NULL;
int optind = 1, opterr = 0, optopt = '\0';

static void getopt_printerr(const char *msg) {
  if (opterr) {
    fprintf(stderr, "%s", msg);
  }
}

static int getopt_in(char d, const char *str) {
  int i = 0;
  while (str[i] != '\0') {
    if (d == str[i] && str[i] != ':') {
      return i;
    }
    i++;
  }
  return 0;
}

static void getopt_exchange(char *argv[], int i, int j) {
  char *tmp = argv[i];
  argv[i] = argv[j];
  argv[j] = tmp;
}

int getopt(int argc, char **argv, const char *optstring) {
  int c;
  static char *nextchar = NULL;
  static int coropt = 1;
  if (!coropt) {
    coropt = 1;
  }

  if (coropt >= argc || argv[coropt] == NULL) {
    return -1;
  }

  while (argv[coropt] && argv[coropt][0] != '-') {
    coropt++;
  }

  if (nextchar == NULL || *nextchar == '\0') {
    if (coropt >= argc) {
      return -1;
    }
    nextchar = &argv[coropt][1];
  }

  int idx;
  if (!((idx = getopt_in(*nextchar, optstring)) >= 0)) {
    getopt_printerr("invalid option\n");
    optopt = *nextchar;
    return '?';
  }

  c = *nextchar++;
  if (*nextchar == '\0') {
    coropt++;
    nextchar = NULL;
  }

  if (optstring[idx + 1] != ':') {
    coropt--;
    optind++;
    goto exit;
  }

  if (nextchar != NULL && *nextchar != '\0') {
    coropt++;
  }

  if (coropt >= argc || argv[coropt][0] == '-') {
    getopt_printerr("option requires an argument\n");
    optopt = *nextchar;
    return '?';
  }

  optarg = argv[coropt];

exit: {
  int i = coropt;
  int j = coropt - 1;
  while (j >= 0 && argv[j][0] != '-') {
    getopt_exchange(argv, i, j);
    i--;
    j--;
  }
}
  optind++;
  return c;
}
