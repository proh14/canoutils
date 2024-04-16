#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>

#define NAME "rm (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "CobbCoding"

char **shift(int *argc, char ***argv);
int rm(char *filename);

#define print_version()                                                        \
  do {                                                                         \
    printf("%s\nversion: %s\nby: %s\n", NAME, VERSION, AUTHOR);               \
  } while (0)
  
char **shift(int *argc, char ***argv) {
  char **result = *argv;
  *argv += 1;
  *argc -= 1;
  return result;
}
  
int rm(char *filename) {
  assert(filename != NULL);
  if(opendir(filename) != NULL) {
    fprintf(stderr, "`%s` is a directory\n", filename);
    exit(1);
  }
  int err = remove(filename);
  if(err == -1) {
    fprintf(stderr, "could not remove file `%s`\n", filename);
    exit(1);
  }
  return err;
}

int main(int argc, char **argv) {
  char *program = *shift(&argc, &argv);
  (void)program;
  char *filename = NULL;
  if(argc == 0) exit(1);
  char *flag = *shift(&argc, &argv);
  if(strcmp(flag, "--version") == 0) {
    print_version();
    return 0;
  } else if(strcmp(flag, "--help") == 0) {
    system("man rm");
  } else {
    filename = flag;
  }
  
  rm(filename);
  return 0;
}
