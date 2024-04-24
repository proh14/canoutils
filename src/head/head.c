#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "head (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "cgetopt.h"
#include "version_info.h"

static const char usage[] = {
    "Usage: head [Option]... [File]...\n"
    "  --version     version information\n"
    "  --help        display this help and exit\n"
    "  -n            number of lines to print form each file\n"
    "  -q, --quiet   do not print file headers\n"};

// flags
static size_t lines = 10;
static int print_header = 0;
static int verbose = 1;

static struct option long_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {"quiet", no_argument, NULL, 'q'},
};

static void print_head(char *path, FILE *file) {
  size_t nl_count = 0;
  int cur_char;

  if (path != NULL && verbose)
    printf("\n==> %s <==\n", path);

  while (nl_count < lines) {
    if ((cur_char = getc(file)) == '\n')
      nl_count++;

    if (cur_char == EOF)
      return;

    putchar(cur_char);
  }
}

int main(int argc, char **argv) {
  int opt, option_index;
  while ((opt = getopt_long(argc, argv, "n:q", long_options, &option_index)) !=
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
    case 'n':
      if (sscanf(optarg, "%lu", &lines) != 1) {
        fprintf(stderr, "head: line count should be a positive integer");
        return EXIT_FAILURE;
      }
      break;
    case 'q':
      verbose = 0;
      break;
    }
  }

  argc -= optind;
  argv += optind;

  int index = 0;

  if (argc < 1) {
    print_head(NULL, stdin);
    return EXIT_SUCCESS;
  }

  if (argc > 1 && verbose)
    print_header = 1;

  for (; index < argc; index++) {
    FILE *file = fopen(argv[index], "r+");

    if (file == NULL) {
      fprintf(stderr, "head: %s: %s\n", argv[index], strerror(errno));
      continue;
    }

    print_head(argv[index], file);
    fclose(file);
  }
}
