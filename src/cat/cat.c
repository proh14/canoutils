#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define NAME "cat (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Akos Szijgyarto (SzAkos04)"

#include "version_info.h"

static const char HELP[] = {
    "Usage: cat [OPTION]... [FILE]...\n"
    "Concatenate FILE(s) to standard output.\n"
    "\n"
    "With no FILE, or when FILE is -, read standard input.\n"
    "\n"
    "  -A, --show-all           equivalent to -vET\n"
    "  -b, --number-nonblank    number nonempty output lines, overrides -n\n"
    "  -e                       equivalent to -vE\n"
    "  -E, --show-ends          display $ at end of each line\n"
    "  -n, --number             number all output lines\n"
    "  -s, --squeeze-blank      suppress repeated empty output lines\n"
    "  -t                       equivalent to -vT\n"
    "  -T, --show-tabs          display TAB characters as ^I\n"
    "  -u                       (ignored)\n"
    "  -v, --show-nonprinting   use ^ and M- notation, except for LFD and TAB\n"
    "      --help        display this help and exit\n"
    "      --version     output version information and exit\n"
    "\n"
    "Examples:\n"
    "  cat f - g  Output f's contents, then standard input, then g's "
    "contents.\n"
    "  cat        Copy standard input to standard output.\n"};

#define print_help()                                                           \
  do {                                                                         \
    printf("%s", HELP);                                                        \
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

#define BUF_MAX 65535 // max length of a buffer in bytes

#define CONST_FUNC __attribute__((const))

typedef enum {
  NumberNonblank = (1 << 0),  // number nonempty output lines, overrides -n
  ShowEnds = (1 << 1),        // display $ at end of each line
  Number = (1 << 2),          // number all output lines
  SqueezeBlank = (1 << 3),    // suppress repeated empty output lines
  ShowTabs = (1 << 4),        // display TAB characters as ^I
  ShowNonprinting = (1 << 5), // use ^ and M- notation, except for LFD and TAB
} Flag;

static const char FLAGLIST[] = "bEnsTv";

int cat(int filec, char **paths, unsigned int flags);
int print_buffer(char *buf, unsigned int flags);
int print_stdin(unsigned int flags);

inline __attribute__((const)) int stridx(const char *str, char c);

void free_paths(int filec, char **paths);

int main(int argc, char **argv) {
  if (argc < 2) {
    // print stdin
    return (print_stdin(0) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  int filec = 0; // file count
  char **paths = (char **)malloc((argc - 1) * sizeof(char *));
  if (!paths) {
    perror("could not allocate memory");
    return EXIT_FAILURE;
  }

  unsigned int flags = 0;

  // parse arguments
  for (int i = 1; i < argc; ++i) {
    int len = strlen(argv[i]);
    if (len > 1 && argv[i][0] == '-') {
      for (int j = 1; j < len; ++j) {
        switch (argv[i][j]) {
        // flags that are multiple flags in one
        case 'A':
          flags |= ShowNonprinting | ShowEnds | ShowTabs;
          break;
        case 'b':
          flags &= ~Number;
          flags |= NumberNonblank;
          break;
        case 'e':
          flags |= ShowNonprinting | ShowEnds;
          break;
        case 'n':
          flags &= ~NumberNonblank;
          flags |= Number;
          break;
        case 't':
          flags |= ShowNonprinting | ShowTabs;
          break;
        case 'u':
          // disable buffering for the output stream
          // NOTE: for some reason this makes the program run slower on my
          // machine. Running the program with the `-u` flag on this source
          // code, the average runtime increases by approximately 10ms
          if (setvbuf(stdout, NULL, _IONBF, 0) != 0) {
            perror("Error setting output buffer mode");
            return EXIT_FAILURE;
          }
          break;
        case '-':
          if (!strcmp(argv[i], "--show-all")) {
            flags |= ShowNonprinting | ShowEnds | ShowTabs;
          } else if (!strcmp(argv[i], "--number-nonblank")) {
            flags &= ~Number;
            flags |= NumberNonblank;
          } else if (!strcmp(argv[i], "--show-ends")) {
            flags |= ShowEnds;
          } else if (!strcmp(argv[i], "--number")) {
            flags &= ~NumberNonblank;
            flags |= Number;
          } else if (!strcmp(argv[i], "--squeeze-blank")) {
            flags |= SqueezeBlank;
          } else if (!strcmp(argv[i], "--show-tabs")) {
            flags |= ShowTabs;
          } else if (!strcmp(argv[i], "--show-nonprinting")) {
            flags |= ShowNonprinting;
          } else if (!strcmp(argv[i], "--help")) {
            print_help();
            free_paths(filec, paths);
            return EXIT_SUCCESS;
          } else if (!strcmp(argv[i], "--version")) {
            print_version();
            free_paths(filec, paths);
            return EXIT_SUCCESS;
          }
          break;
        default: {
          int flag = stridx(FLAGLIST, argv[i][j]);
          if (flag < 0) {
            fprintf(stderr, "cat: invalid option `-%c`\n", argv[i][j]);
            fprintf(stderr, "Try 'cat --help' for more information.\n");
            free_paths(filec, paths);
            return EXIT_FAILURE;
          }
          flags |= 1 << (flag);
          break;
        }
        }
      }
    } else {
      // check if the file is accessible
      if (access(argv[i], F_OK | R_OK) != 0 && strcmp(argv[i], "-") != 0) {
        fprintf(stderr, "cat: file `%s` not found\n", argv[i]);
        free_paths(filec, paths);
        return EXIT_FAILURE;
      }

      paths[filec] = (char *)malloc((strlen(argv[i]) + 1) * sizeof(char));
      if (!paths[filec]) {
        perror("could not allocate memory");
        free_paths(filec, paths);
        return EXIT_FAILURE;
      }

      strcpy(paths[filec], argv[i]);
      filec++;
    }
  }

  paths = (char **)realloc(paths, filec * sizeof(char *));
  if (!paths) {
    perror("could not allocate memory");
    return EXIT_FAILURE;
  }

  if (cat(filec, paths, flags) != 0) {
    free_paths(filec, paths);
    return EXIT_FAILURE;
  }

  free_paths(filec, paths);
  return EXIT_SUCCESS;
}

int cat(int filec, char **paths, unsigned int flags) {
  // print stdin
  if (filec == 0 || !paths) {
    return print_stdin(flags);
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
    int fd = open(paths[i], O_RDONLY);
    if (fd < 0) {
      perror("cat: could not open device");
      return 1;
    }

    // get the size of the file
    struct stat st;
    if (fstat(fd, &st) < 0) {
      perror("cat: could not get file size");
      close(fd);
      return 1;
    }
    off_t file_size = st.st_size;

    char buf[file_size + 1];
    ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
    if (bytes_read != file_size) {
      perror("cat: could not read file");
      close(fd);
      return 1;
    }

    buf[bytes_read] = '\0'; // null-terminate the string

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
  } else if ((flags & NumberNonblank)) {
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
  ssize_t bytes_read;

  // Read from stdin using a loop to handle large inputs
  while ((bytes_read = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
    if (print_buffer(buf, flags) != 0) {
      return 1;
    }
    // clear the buffer before new prompt
    memset(buf, 0, sizeof(buf));
  }

  if (bytes_read == -1) {
    perror("cat: read error");
    return 1;
  }

  return 0;
}

inline CONST_FUNC int stridx(const char *str, char c) {
  const char *p = strchr(str, c);
  return (p) ? (int)(p - str) : -1;
}

void free_paths(int filec, char **paths) {
  for (int i = 0; i < filec; i++) {
    free(paths[i]);
  }
  free(paths);
}
