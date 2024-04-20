#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define NAME "sleep (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Eike Flath"

#include "version_info.h"

static unsigned int parse_int(char *arg) {
  unsigned int n = 0;
  for (int i = 0; arg[i]; i++) {
    if (!isdigit(arg[i])) {
      fprintf(stderr, "sleep: invalid time interval '%s'\n", arg);
      exit(EXIT_FAILURE);
    }
    unsigned int new_n = n * 10 + (arg[i] - '0');
    if (new_n < n) { // overflow
      fprintf(stderr, "sleep: time interval '%s' is too big\n", arg);
      exit(EXIT_FAILURE);
    }
    n = new_n;
  }
  return n;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "sleep: expected exactly one operand, got %d\n", argc - 1);
    return EXIT_FAILURE;
  }
  if (!strcmp(argv[1], "--version")) {
    print_version();
    return EXIT_SUCCESS;
  }
  sleep(parse_int(argv[1]));
  return EXIT_SUCCESS;
}
