#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "cat (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "SzAkos04"

#define print_version() \
    do { \
        printf("%s\nversion: %s\nby: %s\n", NAME, VERSION, AUTHOR); \
    } while (0)

#define BUF_SIZE 1024

char **shift(int *argc, char ***argv);
int cat(char *path);

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "not enough args\n");
    fprintf(stderr, "see cat --help\n");
    exit(1);
  }
  char *program = *shift(&argc, &argv);
  (void)program;
  char *flag = *shift(&argc, &argv);
  char *path = (char *)malloc(sizeof(char) * BUF_SIZE);
  if (!path) {
    perror("could not allocate memory");
    exit(1);
  }
  if (strcmp(flag, "--version") == 0) {
    print_version();
  } else {
    strcpy(path, flag);
  }

  if (cat(path) != 0) {
    free(path);
    exit(1);
  }

  free(path);
  return 0;
}

char **shift(int *argc, char ***argv) {
  char **result = *argv;
  *argv += 1;
  *argc -= 1;

  return result;
}

int cat(char *path) {
  if (access(path, F_OK | R_OK) != 0) {
    fprintf(stderr, "file does not exist\n");
    return 1;
  }

  FILE *infile = fopen(path, "r");
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

  // read the file
  size_t files_read = fread(buf, sizeof(char), file_size, infile);
  if (files_read != (size_t)file_size) {
    fprintf(stderr, "could not read file\n");
    fclose(infile);
    free(buf);
    return 1;
  }

  fclose(infile);

  for (int i = 0; i < file_size; ++i) {
    if (isprint(buf[i]) || isspace(buf[i])) {
      putchar(buf[i]);
    }
  }

  free(buf);
  return 0;
}

