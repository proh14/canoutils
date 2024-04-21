#include "./getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *optarg = NULL;
int optind = 1, opterr = 0, optopt = '\0';
static int end_of_args = 1;

#define EXCHANGE(coropt)                                                       \
  do {                                                                         \
    int i = (coropt);                                                          \
    int j = (coropt)-1;                                                        \
    while (j >= 0 && argv[j][0] != '-') {                                      \
      getopt_exchange(argv, i, j);                                             \
      i--;                                                                     \
      j--;                                                                     \
    }                                                                          \
  } while (0)

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
  return -1;
}

static void getopt_exchange(char *argv[], int i, int j) {
  char *tmp = argv[i];
  argv[i] = argv[j];
  argv[j] = tmp;
}

int getopt(int argc, char *argv[], const char *optstring) {
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

  if (coropt >= argc) {
    return -1;
  }

  nextchar = &argv[coropt][1];

  if (strcmp(argv[coropt], "--") == 0) {
    EXCHANGE(coropt);
    end_of_args = 0;
    coropt++;
    return -1;
  }

  int idx;
  if (!((idx = getopt_in(*nextchar, optstring)) >= 0)) {
    getopt_printerr("invalid option\n");
    optopt = *nextchar;
    if (*(nextchar + 1) == '\0') {
      nextchar = NULL;
      optind++;
    }
    c = '?';
    goto exit;
  }

  c = *nextchar++;
  if (*nextchar == '\0') {
    coropt++;
    optind++;
    nextchar = argv[coropt];
  }

  if (optstring[idx + 1] != ':') {
    coropt--;
    goto exit;
  }
  EXCHANGE(coropt - 1);

  if (nextchar != NULL && *nextchar != '\0') {
    coropt++;
  }

  if (coropt >= argc || argv[coropt][0] == '-') {
    getopt_printerr("option requires an argument\n");
    optopt = *nextchar;
    c = '?';
    goto exit;
  }

  optarg = argv[coropt];
  optind++;

exit : { EXCHANGE(coropt); }
  return c;
}

int getopt_long(int argc, char *argv[], const char *optstring,
                const struct option *longopts, int *longindex) {
  (void)longindex;
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

  if (*nextchar == '-') {
    nextchar++;
    while (longopts->name != NULL) {
      if (strcmp(longopts->name, nextchar) == 0) {
        optind++;
        nextchar = NULL;
        c = longopts->val;
        if (longopts->has_arg) {
          EXCHANGE(coropt);
          coropt++;
          if (coropt >= argc || argv[coropt][0] == '-') {
            getopt_printerr("option requires an argument\n");
            optopt = longopts->val;
            c = '?';
            goto exit;
          }
          optarg = argv[coropt];
          optind++;
        }
        goto exit;
      }
      longopts++;
    }
    optind++;
    optopt = longopts->val;
    nextchar = NULL;
    getopt_printerr("invalid option\n");
    c = '?';
    goto exit;
  }

  int idx;
  if (!((idx = getopt_in(*nextchar, optstring)) >= 0)) {
    getopt_printerr("invalid option\n");
    optopt = *nextchar;
    if (*(nextchar + 1) == '\0') {
      nextchar = NULL;
      optind++;
    }
    c = '?';
    goto exit;
  }

  c = *nextchar++;
  if (*nextchar == '\0') {
    coropt++;
    nextchar = NULL;
    optind++;
  }

  if (optstring[idx + 1] != ':') {
    coropt--;
    goto exit;
  }
  EXCHANGE(coropt - 1);

  if (nextchar != NULL && *nextchar != '\0') {
    coropt++;
  }

  if (coropt >= argc || argv[coropt][0] == '-') {
    getopt_printerr("option requires an argument\n");
    optopt = *nextchar;
    c = '?';
    goto exit;
  }

  optarg = argv[coropt];
  optind++;

exit : { EXCHANGE(coropt); }
  return c;
}
