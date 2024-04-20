#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "pwd (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Mehadav Itay (whiteapolo)"

#define print_version()                                                        \
  do {                                                                         \
    printf("%s\nversion: %s\nby: %s\n", NAME, VERSION, AUTHOR);                \
  } while (0)

char *realpath(const char *path, char *resolved_path);

int status = 0;
bool logical = false;

char buff[256];

void print_pwd_pysical() {
  getcwd(buff, 256);
  printf("%s\n", buff);
}

void print_pwd_logical() {
  char *env = getenv("PWD");
  if (env == NULL) {
    printf("pwd: cannot access PWD varibale\n");
    status = 1;
    return;
  }
  printf("%s\n", env);
}

void print_file_path(const char *file) {
  if (access(file, F_OK)) {
    printf("pwd: cannot access '%s': No such file or directory\n", file);
    return;
    status = 1;
  }

  realpath(file, buff);
  printf("%s\n", buff);
}

void print_help() {
  puts(" -L, get pwd from environment varible");
  puts(" --version, output version information and exit");
  puts(" --help, display this help and exit");
}

int main(int argc, char **argv) {
  int option_count = 0;
  for (int i = 1; i < argc; i++) {
    if (*argv[i] == '-') {
      if (argv[i][1] == '-') {
        if (!strcmp(argv[i], "--version")) {
          print_version();
          return 0;
        } else if (!strcmp(argv[i], "--help")) {
          print_help();
          return 0;
        } else {
          printf("pwd: unrecognized option '%s'\n", argv[i]);
          return 1;
        }
      } else {
        switch (argv[i][1]) {
        case 'L':
          logical = true;
          break;
        }
      }
      option_count++;
    }
  }

  if (option_count == argc - 1) {
    if (logical)
      print_pwd_logical();
    else
      print_pwd_pysical();
    return status;
  }

  for (int i = 1; i < argc; i++) {
    if (*argv[i] != '-') {
      print_file_path(argv[i]);
    }
  }
}
