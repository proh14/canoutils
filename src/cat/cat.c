#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define NAME "cat (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "SzAkos04"

#define print_version() \
    do { \
      printf("%s\nversion: %s\nby: %s\n", NAME, VERSION, AUTHOR); \
    } while (0)

#define print_help() \
  do { \
    printf("Usage: cat [OPTION]... [FILE]...\n"); \
    printf("Concatenate FILE(s) to standard output.\n"); \
  } while (0)

#define print_incorrect_args() \
  do { \
    printf("incorrect args\n"); \
    printf("see `cat --help`\n"); \
  } while (0)

#define PATH_LEN 256
#define ARGS_MAX 16 // number of the max arguments
#define ARGS_LEN 32

int cat(int filec, char **paths, int argc, char **argv);

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "not enough args\n");
    fprintf(stderr, "see `cat --help`\n");
    exit(1);
  }

  if (strcmp(argv[1], "--version") == 0) {
    if (argc != 2) {
      print_incorrect_args();
      exit(1);
    }

    print_version();
    return 0;
  }

  if (strcmp(argv[1], "--help") == 0) {
    if (argc != 2) {
      print_incorrect_args();
      exit(1);
    }

    print_help();
    return 0;
  }

  int filec = 0; // file count
  char **paths = (char **)malloc(sizeof(char) * PATH_LEN * ARGS_MAX);
  if (!paths) {
    perror("could not allocate memory");
    exit(1);
  }

  // arguments for the `cat` function
  int cat_argc = 0;
  char **cat_argv = (char **)malloc(sizeof(char) * ARGS_LEN * ARGS_MAX);
  if (!cat_argv) {
    perror("could not allocate memory");
    free(paths);
    exit(1);
  }

  // parse arguments
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "--show-ends") == 0
        || strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "--show-tabs") == 0) {
      cat_argv[cat_argc++] = argv[i];
    } else {
      if (access(argv[i], F_OK | R_OK) != 0) {
        fprintf(stderr, "file `%s` not found\n", argv[i]);
        free(paths);
        free(cat_argv);
        exit(1);
      }

      paths[filec++] = argv[i];
    }
  }

  if (cat(filec, paths, cat_argc, cat_argv) != 0) {
    free(paths);
    free(cat_argv);
    exit(1);
  }

  free(paths);
  free(cat_argv);

  return 0;
}

int cat(int filec, char **paths, int argc, char **argv) {
  // cat arguments
  /* bool A, b, e, E, n, s, t, T, u, v; */
  bool E = false; // show ends
  bool T = false; // show tabs
  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "--show-ends") == 0) {
      E = true;
    }
    if (strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "--show-tabs") == 0) {
      T = true;
    }
  }

  for (int i = 0; i < filec; ++i) {
    FILE *infile = fopen(paths[i], "r");
    if (!infile) {
      perror("could not open file");
      return 1;
    }

    int ch;
    while ((ch = fgetc(infile)) != EOF) {
      if (E && ch == '\n') {
        putchar('$');
      }
      if (T && ch == '\t') {
        puts("^I");
        continue;
      }
      putchar(ch);
    }

    fclose(infile);
  }
  return 0;
}

