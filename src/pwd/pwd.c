#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "pwd (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "cgetopt.h"
#include "version_info.h"

static const char usage[] = {"Usage: pwd [Option]...\n"
                             "  --version   version information\n"
                             "  --help      display this help and exit\n"
                             "  -L          print contents of PWD if it is the "
                             "current directory (default)\n"
                             "  -P          path written to standard out shall "
                             "not have any symbolic names in it\n"};

// flags
static int output_format = 'L';

static struct option long_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
};

int main(int argc, char **argv) {

  int opt, option_index;
  while ((opt = getopt_long(argc, argv, "LP", long_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 0:
      break;
    case 'v':
      print_version();
      return EXIT_SUCCESS;
      break;
    case 'h':
      printf("%s\n", usage);
      return EXIT_SUCCESS;
    case 'L':
      output_format = 'L';
      break; // TODO implement
    case 'P':
      output_format = 'P';
      break; // TODO
    }
  }

  argc -= optind;
  argv += optind;

  if (argc > 0) {
    fprintf(stderr, "%s\n", usage);
    return EXIT_FAILURE;
  }

  char *path = getenv("PWD");
  printf("%s\n", path);
}
