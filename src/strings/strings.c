#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "strings (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "cgetopt.h"
#include "version_info.h"

static const char usage[] = {
    "Usage: strings [Option]... [File]...\n"
    "  --version   version information\n"
    "  --help      display this help and exit\n"
    "  -a          scan files in their entirety\n"
    "  -n NUMBER   specify minimum string length\n"
    "  -t FORMAT   write each string with its byte offset\n"
    "                o  offset is in octal\n"
    "                x  offset is in hex\n"
    "                d  offset is in decimal\n"};

// flags
static int long_output = 0;
static int output_format = 't';
static size_t string_length = 4;

static struct option long_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
};

static void strings(FILE *file) {
  size_t offset = 0;
  size_t ngraph = 0;
  int cur_char;
  char *buf = malloc(string_length + 1);
  buf[string_length] = 0;

  while ((cur_char = getc(file)) != EOF) {
    if ((isgraph(cur_char) || isspace(cur_char)) && cur_char != '\n') {
      buf[ngraph] = cur_char;
      ngraph++;
    }

    if (ngraph > string_length) {
      if (long_output) {
        switch (output_format) {
        case 'o':
          printf("%lo ", offset);
          break;
        case 'x':
          printf("%lx ", offset);
          break;
        case 'd':
          printf("%lu ", offset);
          break;
        }
      }

      printf("%s", buf);
      while ((isgraph(cur_char = getc(file)) || isspace(cur_char)) &&
             cur_char != '\n') {
        putchar(cur_char);
        offset++;
      }
      putchar('\n');
      ngraph = 0;
    }

    offset++;
  }

  free(buf);
}

int main(int argc, char **argv) {
  int opt, option_index;
  while ((opt = getopt_long(argc, argv, "an:t:", long_options,
                            &option_index)) != -1) {
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
    case 'a': // default
      break;
    case 'n':
      if (sscanf(optarg, "%lu", &string_length) != 1) {
        fprintf(stderr, "strings: -n: expected positive decimal number\n");
        return EXIT_FAILURE;
      }
      break;
    case 't':
      if (optarg[1] != '\0') {
        fprintf(stderr,
                "strings: -t: expected one of 'o', 'x', 'd'. got `%s`\n",
                optarg);
        return EXIT_FAILURE;
      }
      switch (optarg[0]) {
      case 'o':
      case 'x':
      case 'd':
        output_format = optarg[0];
        break;
      default:
        fprintf(stderr,
                "strings: -t: expected one of 'o', 'x', 'd'. got `%s`\n",
                optarg);
        return EXIT_FAILURE;
      }
      long_output = 1;
      break;
    }
  }

  argc -= optind;
  argv += optind;

  int index = 0;

  if (argc < 1) {
    strings(stdin);
    return EXIT_SUCCESS;
  }

  for (; index < argc; index++) {
    FILE *file = fopen(argv[index], "r+");

    if (file == NULL) {
      fprintf(stderr, "strings: %s: %s\n", argv[index], strerror(errno));
      continue;
    }

    strings(file);
    fclose(file);
  }
}
