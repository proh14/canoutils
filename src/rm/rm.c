#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "rm (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "CobbCoding"

char **shift(int *argc, char ***argv);

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

int main(int argc, char **argv) {
  char *program = *shift(&argc, &argv);
  (void)program;
  char *filename = NULL;
  if(argc == 0) exit(1);
  char *flag = *shift(&argc, &argv);
  if(strcmp(flag, "--version") == 0) {
    print_version();
  } else {
    filename = flag;
    printf("%s\n", filename);
  }
  return 0;
}
