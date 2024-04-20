#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "sleep (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Eike Flath"

#include "version_info.h"

static void handle_sigalrm(int sig) {
  (void)sig;
  exit(EXIT_SUCCESS);
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
  char *endptr = NULL;
  long n = strtol(argv[1], &endptr, 10);
  if (argv[1][0] == '\0' || *endptr != '\0') {
    fprintf(stderr, "sleep: invalid time interval '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }
  if (n < 0) {
    fprintf(stderr, "sleep: please provide a non-negative time interval\n");
    return EXIT_FAILURE;
  }
  if (n > UINT_MAX) {
    fprintf(stderr, "sleep: time interval '%s' is too big\n", argv[1]);
    return EXIT_FAILURE;
  }
  signal(SIGALRM, handle_sigalrm);
  return sleep((unsigned int)n) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
