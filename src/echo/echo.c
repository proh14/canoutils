#include <stdio.h>
#include <string.h>

#define NAME "echo (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Hoorad Farrokh (proh14)"

#define print_version()                                                        \
  do {                                                                         \
    printf("%s\nversion: %s\nby: %s\n", NAME, VERSION, AUTHOR);                \
  } while (0)

int main(int argc, char **argv) {
  int escape = 0;
  int new_line = 1;
  int i = 1;
  if (argc == 1) {
    printf("\n");
    return 0;
  }

  if (strcmp(argv[1], "--version") == 0) {
    print_version();
    return 0;
  }

  if (strcmp(argv[1], "-e") == 0) {
    escape = 1;
    i++;
  }

  if (strcmp(argv[1], "-n") == 0) {
    new_line = 0;
    i++;
  }

  while (argv[i] != NULL) {
    for (int j = 0; argv[i][j] != '\0'; j++) {
      if (escape && argv[i][j] == '\\') {
        switch (argv[i][j + 1]) {
        case '\\':
          putchar('\\');
          break;
        case 'a':
          putchar('\a');
          break;
        case 'b':
          putchar('\b');
          break;
        case 'c':
          return 0;
          break;
        case 'e':
          putchar('\x1b');
          break;
        case 'f':
          putchar('\f');
          break;
        case 'n':
          putchar('\n');
          break;
        case 'r':
          putchar('\r');
          break;
        case 't':
          putchar('\t');
          break;
        case 'v':
          putchar('\v');
          break;
        default:
          break;
        }
        if (argv[i][j + 1] != '\0')
          j++;
      } else {
        putchar(argv[i][j]);
      }
    }
    if (argv[i + 1] != NULL) {
      putchar(' ');
    }
    i++;
  }

  if (new_line) {
    putchar('\n');
  }

  return 0;
}
