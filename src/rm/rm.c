#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "rm (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "CobbCoding"

typedef enum {
  VERBOSE,
  FLAG_COUNT,
} Flag;

typedef struct {
  Flag data[FLAG_COUNT];
  size_t count;
} Flags;

char **shift(int *argc, char ***argv);
int rm(char *filename, Flags flags);

#define print_version()                                                        \
  do {                                                                         \
    printf("%s\nversion: %s\nby: %s\n", NAME, VERSION, AUTHOR);                \
  } while (0)

#define print_not_enough()                                                     \
  do {                                                                         \
    fprintf(stderr, "not enough args\n");                                      \
    fprintf(stderr, "see rm --help\n");                                        \
    exit(1);                                                                   \
  } while (0)

char **shift(int *argc, char ***argv) {
  char **result = *argv;
  *argv += 1;
  *argc -= 1;
  return result;
}

int rm(char *filename, Flags flags) {
  assert(filename != NULL);
  if (opendir(filename) != NULL) {
    fprintf(stderr, "`%s` is a directory\n", filename);
    exit(1);
  }
  int err = remove(filename);
  if (err == -1) {
    fprintf(stderr, "could not remove file `%s`\n", filename);
    exit(1);
  }

  for (size_t i = 0; i < flags.count; i++) {
    switch (flags.data[i]) {
    case VERBOSE:
      printf("removing `%s`\n", filename);
      break;
    default:
      continue;
    }
  }

  return err;
}

int main(int argc, char **argv) {
  char *program = *shift(&argc, &argv);
  (void)program;
  char *filename = NULL;
  if (argc == 0) {
    fprintf(stderr, "not enough args\n");
    fprintf(stderr, "see rm --help\n");
    exit(1);
  }

  if (argc == 0) {
    print_not_enough();
  }

  Flags flags = {0};

  char *flag = *shift(&argc, &argv);
  if (strcmp(flag, "--version") == 0) {
    print_version();
    return 0;
  } else if (strcmp(flag, "--help") == 0) {
    system("man rm");
    return 0;
  } else if (strcmp(flag, "-v") == 0) {
    flags.data[flags.count++] = VERBOSE;
    if (argc == 0) {
      print_not_enough();
    }
    filename = *shift(&argc, &argv);
  } else {
    filename = flag;
  }

  while (argc >= 0) {
    rm(filename, flags);
    filename = *shift(&argc, &argv);
  }
  return 0;
}
