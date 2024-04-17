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

#define BUF_MAX_LEN 4096
#define PATH_LEN 256
#define ARGS_MAX 16 // number of the max arguments
#define ARGS_LEN 32

bool E = false; // show ends
bool n = false; // number
bool T = false; // show tabs

int cat(int filec, char **paths);
int print_file(char *buf);

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

  // parse arguments
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "--show-ends") == 0) {
      E = true;
      continue;
    }
    if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--number") == 0) {
      n = true;
      printf(" 1  ");
      continue;
    }
    if (strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "--show-tabs") == 0) {
      T = true;
      continue;
    } else {
      if (access(argv[i], F_OK | R_OK) != 0 && strcmp(argv[i], "-") != 0) {
        fprintf(stderr, "file `%s` not found\n", argv[i]);
        free(paths);
        exit(1);
      }

      paths[filec++] = argv[i];
      continue;
    }
  }

  if (cat(filec, paths) != 0) {
    free(paths);
    exit(1);
  }

  free(paths);

  return 0;
}

int cat(int filec, char **paths) {
  for (int i = 0; i < filec; ++i) {
    // read from stdin
    if (strcmp(paths[i], "-") == 0) {
      char *buf = (char *)malloc(sizeof(char) * BUF_MAX_LEN);
      scanf("%s", buf);
      if (!buf) {
        perror("could not allocate memory");
        return 1;
      }

      if (print_file(buf) != 0) {
        free(buf);
        return 1;
      }
      free(buf);

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

    char *buf = (char *)malloc(sizeof(char) * (file_size + 1));
    if (!buf) {
      perror("could not allocate memory");
      fclose(infile);
      return 1;
    }

    // read the file into the buffer
    size_t files_read = fread(buf, sizeof(char), file_size, infile);
    if (files_read != (size_t)file_size) {
      fprintf(stderr, "could not read file\n");
      fclose(infile);
      free(buf);
      return 1;
    }
    buf[file_size] = '\0';

    fclose(infile);

    if (print_file(buf) != 0) {
      return 1;
    }

    free(buf);
  }
  return 0;
}

int print_file(char *buf) {
  int lines = 1;
  for (size_t i = 0; i < strlen(buf); ++i) {
    if (E && buf[i] == '\n') {
      putchar('$');
    }
    if (n && buf[i] == '\n') {
      // FIXME: this adds one more line on the end of the file
      printf("\n %i  ", ++lines);
      continue;
    }
    if (T && buf[i] == '\t') {
      puts("^I");
      continue;
    }

    putchar(buf[i]);
  }
  putchar('\n');

  return 0;
}
