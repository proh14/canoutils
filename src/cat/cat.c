#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "cat (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Akos Szijgyarto (SzAkos04)"

#include "version_info.h"

#define print_help()                                                           \
  do {                                                                         \
    printf("Usage: cat [OPTION]... [FILE]...\n");                              \
    printf("Concatenate FILE(s) to standard output.\n");                       \
  } while (0)

#define print_incorrect_args()                                                 \
  do {                                                                         \
    printf("incorrect arguments\n");                                           \
    printf("see `cat --help`\n");                                              \
  } while (0)

#define assert_argc(argc, n)                                                   \
  do {                                                                         \
    if (argc != n) {                                                           \
      print_incorrect_args();                                                  \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#define BUF_MAX 4096 // max length of a buffer in bytes
#define PATH_MAX 256 // max length of a path in bytes
#define ARGS_MAX 16  // number of the max arguments
#define ARGS_LEN 32  // max length of the arguments in bytes

typedef enum {
  NumberNonblank = (1 << 0),  // number nonempty output lines, overrides -n
  ShowEnds = (1 << 1),        // display $ at end of each line
  Number = (1 << 2),          // number all output lines
  SqueezeBlank = (1 << 3),    // suppress repeated empty output lines
  ShowTabs = (1 << 4),        // display TAB characters as ^I
  ShowNonprinting = (1 << 5), // use ^ and M- notation, except for LFD and TAB
} Flag;

int cat(int filec, char **paths, unsigned int flags);
int print_buffer(char *buf, unsigned int flags);
int print_stdin(unsigned int flags);

int main(int argc, char **argv) {
  if (argc < 2) {
    // print stdin
    return (print_stdin(0) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  if (!strcmp(argv[1], "--version")) {
    assert_argc(argc, 2);

    print_version();
    return EXIT_SUCCESS;
  }

  if (!strcmp(argv[1], "--help")) {
    assert_argc(argc, 2);

    print_help();
    return EXIT_SUCCESS;
  }

  int filec = 0; // file count
  char *paths[ARGS_MAX];

  unsigned int flags = 0;

  // parse arguments
  for (int i = 1; i < argc; ++i) {
    int len = strlen(argv[i]);
    if (len > 1 && argv[i][0] == '-') {
      for (int j = 1; j < len; ++j) {
        switch (argv[i][j]) {
        case 'A':
          flags |= ShowNonprinting;
          flags |= ShowEnds;
          flags |= ShowTabs;
          continue;
        case 'b':
          flags &= ~Number;
          flags |= NumberNonblank;
          continue;
        case 'e':
          flags |= ShowNonprinting;
          flags |= ShowEnds;
          continue;
        case 'E':
          flags |= ShowEnds;
          continue;
        case 'n':
          flags &= ~NumberNonblank;
          flags |= Number;
          continue;
        case 's':
          flags |= SqueezeBlank;
          continue;
        case 't':
          flags |= ShowNonprinting;
          flags |= ShowTabs;
          continue;
        case 'T':
          flags |= ShowTabs;
          continue;
        case 'v':
          flags |= ShowNonprinting;
          continue;
        case '-':
          if (!strcmp(argv[i], "--show-all")) {
            flags |= ShowNonprinting;
            flags |= ShowEnds;
            flags |= ShowTabs;
          } else if (!strcmp(argv[i], "--number-nonblank")) {
            flags |= NumberNonblank;
            flags &= ~Number;
          } else if (!strcmp(argv[i], "--show-ends")) {
            flags |= ShowEnds;
          } else if (!strcmp(argv[i], "--number")) {
            flags |= Number;
            flags &= ~NumberNonblank;
          } else if (!strcmp(argv[i], "--squeeze-blank")) {
            flags |= SqueezeBlank;
          } else if (!strcmp(argv[i], "--show-tabs")) {
            flags |= ShowTabs;
          } else if (!strcmp(argv[i], "--show-nonprinting")) {
            flags |= ShowNonprinting;
          }
          break;
        default:
          fprintf(stderr, "unknown argument `%s`", argv[i]);
          return EXIT_FAILURE;
        }
      }
    } else {
      // check if the file is accessible
      if (access(argv[i], F_OK | R_OK) != 0 && strcmp(argv[i], "-") != 0) {
        fprintf(stderr, "file `%s` not found\n", argv[i]);
        return EXIT_FAILURE;
      }

      paths[filec++] = argv[i];
    }
  }

  return (cat(filec, paths, flags) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int cat(int filec, char **paths, unsigned int flags) {
  // print stdin
  if (filec == 0 || !paths) {
    return (print_stdin(flags) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  for (int i = 0; i < filec; ++i) {
    // print stdin
    if (!strcmp(paths[i], "-")) {
      if (print_stdin(flags) != 0) {
        return 1;
      }

      continue;
    }

    // read file
    FILE *infile = fopen(paths[i], "r");
    if (!infile) {
      perror("could not open file");
      return 1;
    }

    // get the size of the file
    fseek(infile, 0, SEEK_END);
    long file_size = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    char buf[file_size + 1];

    // read the file into the buffer
    size_t files_read = fread(buf, sizeof(char), file_size, infile);
    if (files_read != (size_t)file_size) {
      fprintf(stderr, "could not read file\n");
      fclose(infile);
      return 1;
    }
    buf[file_size] = '\0'; // make sure the string is null terminated

    fclose(infile);

    if (print_buffer(buf, flags) != 0) {
      return 1;
    }
  }
  return 0;
}

#define BEFORE_NUMBER 6 // number of spaces before line numbers

int print_buffer(char *buf, unsigned int flags) {
  int lines = 1;
  if ((flags & Number)) {
    // print number before the first line
    printf("%*d  ", BEFORE_NUMBER, lines);
  }
  if ((flags & NumberNonblank)) {
    if (buf[0] != '\n' && buf[1] != '\0') {
      // print number before the first line
      printf("%*d  ", BEFORE_NUMBER, lines);
    }
  }
  int len = strlen(buf);
  for (int i = 0; i < len; ++i) {
    // higher priority
    // NOTE: not the prettiest code, but it works
    if ((flags & SqueezeBlank) && buf[i] == '\n') {
      // skip over consecutive '\n' characters
      if (i + 1 < len && buf[i + 1] == '\n') {
        // if the consecutive '\n' characters are over
        if (i + 2 < len && buf[i + 2] != '\n') {
          if ((flags & Number)) {
            printf("\n%*d  ", BEFORE_NUMBER, ++lines);
          } else {
            putchar('\n');
          }
        }
        continue;
      }
    }

    if ((flags & NumberNonblank) && buf[i] == '\n' && buf[i + 1] != '\n' &&
        buf[i + 1] != '\0') {
      printf("\n%*d  ", BEFORE_NUMBER, ++lines);
      continue;
    }
    if ((flags & ShowEnds) && buf[i] == '\n') {
      putchar('$');
    }
    if ((flags & Number) && buf[i] == '\n' && buf[i + 1] != '\0') {
      printf("\n%*d  ", BEFORE_NUMBER, ++lines);
      continue;
    }
    if ((flags & ShowTabs) && buf[i] == '\t') {
      printf("^I");
      continue;
    }
    if ((flags & ShowNonprinting) && !isprint(buf[i]) && buf[i] != 9 &&
        buf[i] != 10) {
      if (buf[i] & 0x80) {
        // meta (M-) notation for characters with the eighth bit set
        printf("M-");
        char printable_char = buf[i] & 0x7F; // clear the eighth bit
        printf("^%c", '@' + printable_char);
      } else {
        // regular non-printable character notation
        printf("^%c", '@' + buf[i]);
      }
      continue;
    }

    putchar(buf[i]);
  }

  return 0;
}

int print_stdin(unsigned int flags) {
  char buf[BUF_MAX];
  while (fgets(buf, BUF_MAX, stdin)) {
    if (print_buffer(buf, flags) != 0) {
      return 1;
    }
  }

  return 0;
}
