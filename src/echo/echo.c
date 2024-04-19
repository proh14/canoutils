#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "echo (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Hoorad Farrokh (proh14)"

#include "version_info.h"

int isoctal(int c) { return (c >= '0' && c <= '7'); }

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

  if (strcmp(argv[1], "-en") == 0 || strcmp(argv[1], "-ne") == 0) {
    escape = 1;
    new_line = 0;
    i++;
  }

  if (strcmp(argv[1], "--help") == 0) {
    system("man echo");
    return 0;
  }

  while (argv[i] != NULL) {
    for (int j = 0; argv[i][j] != '\0'; j++) {
      if (escape && argv[i][j] == '\\') {
        int add = 1;
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
        case 'x':
          if (argv[i][j + 2] == '\0') {
            break;
          }
          char hex[3] = {argv[i][j + 2], argv[i][j + 3], '\0'};
          if (isxdigit(hex[0])) {
            add = 3;
            if (!isxdigit(hex[1])) {
              hex[1] = '\0';
              add--;
              break;
            }
            unsigned value = 0;
            sscanf(hex, "%x", &value);
            putchar((int)value);
          }
          break;
        case '0':
          if (argv[i][j + 2] == '\0') {
            break;
          }
          char octal[4] = {argv[i][j + 2], argv[i][j + 3], argv[i][j + 4],
                           '\0'};
          if (isoctal(octal[0])) {
            add = 4;
            if (!isoctal(octal[1])) {
              hex[1] = '\0';
              add -= 2;
            } else if (!isoctal(octal[2])) {
              hex[2] = '\0';
              add--;
            }
            unsigned value = 0;
            sscanf(octal, "%o", &value);
            putchar((int)value);
          }
          break;
        default:
          add = 0;
          putchar(argv[i][j]);
          break;
        }
        if (argv[i][j + 1] != '\0')
          j += add;
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
