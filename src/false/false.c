#include <stdio.h>
#include <stdlib.h>

#define NAME "false (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "cgetopt.h"
#include "version_info.h"

static const char usage[] = {"Usage: false\n"
                             "  --version   version information\n"
                             "  --help      display this help and exit\n"};

static struct option long_options[] = {{"version", no_argument, NULL, 'v'},
                                       {"help", no_argument, NULL, 'h'}};

int main(int argc, char **argv) {
  int opt, option_index;
  while ((opt = getopt_long(argc, argv, "", long_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 0:
      break;
    case 'v':
      print_version();
      return EXIT_FAILURE;
    case 'h':
      printf("%s\n", usage);
      return EXIT_FAILURE;
    }
  }

  argc -= optind;
  argv += optind;

  return EXIT_FAILURE;
}
