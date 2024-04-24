#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "basename (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "cgetopt.h"
#include "version_info.h"

static const char usage[] = {"Usage: basename [Options]... NAMES [suffix]\n"
                             "  --version   version information\n"
                             "  --help      display this help and exit\n"
                             "  -z          print zero instead of newline\n"
                             "  -a          multiple NAMES and no suffix\n"};

// flags
static int print_zero = 0;
static int multiple = 0;

static struct option long_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
};

static char *basename(char *str, char *suffix) {
  char *end = str + strlen(str), *start = str;

  /* remove trailing '/' */
  for (; --end > start && *end == '/';)
    *end = 0;

  /* remove prefix ending with '/' */
  for (; start < end; start++)
    if (*start == '/')
      str = start + 1;

  /* remove suffix */
  size_t sufflen = strlen(suffix);
  if (end + 1 - sufflen >= str && strcmp(suffix, end + 1 - sufflen) == 0)
    *(end + 1 - sufflen) = 0;

  return str;
}

int main(int argc, char **argv) {
  int opt, option_index;
  while ((opt = getopt_long(argc, argv, "za", long_options, &option_index)) !=
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
    case 'z':
      print_zero = 1;
      break;
    case 'a':
      multiple = 1;
      break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc < 1 || (!multiple && argc > 2)) {
    printf("%s\n", usage);
    return EXIT_FAILURE;
  }

  char *suffix = multiple ? "" : argv[1] == NULL ? "" : argv[1];
  char *string = argv[0];

  char *base = basename(string, suffix);
  if (print_zero) {
    printf("%s", base);
    putchar(0);
  } else {
    printf("%s\n", base);
  }

  if (multiple) {
    int index = 1;
    while (index < argc) {
      string = argv[index];
      base = basename(string, suffix);
      if (print_zero) {
        printf("%s", base);
        putchar(0);
      } else {
        printf("%s\n", base);
      }

      index++;
    }
  }
}
