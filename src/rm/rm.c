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

bool verbose = false;
bool prompt_every = false;
bool remove_dir = false;

char **shift(int *argc, char ***argv);
int rm(char *filename);

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
  
#define shift_flags()  \
  do {  \
  if (argc == 0) {  \
    print_not_enough();  \
  }  \
  flag = *shift(&argc, &argv);  \
  filename = flag;  \
  } while(0)

char **shift(int *argc, char ***argv) {
  char **result = *argv;
  *argv += 1;
  *argc -= 1;
  return result;
}

int rm(char *filename) {
  assert(filename != NULL);
  if(!remove_dir) {
    if (opendir(filename) != NULL) {
      fprintf(stderr, "`%s` is a directory\n", filename);
      exit(1);
    }
  }
  int err = remove(filename);
  if (err == -1) {
    fprintf(stderr, "could not remove file `%s`\n", filename);
    exit(1);
  }

  if(verbose) {
    printf("removing `%s`\n", filename); 
  }
    
  return err;
}

int main(int argc, char **argv) {
  char *program = *shift(&argc, &argv);
  (void)program;
  char *filename = NULL;
  char *flag = NULL;
  if (argc == 0) {
    fprintf(stderr, "not enough args\n");
    fprintf(stderr, "see rm --help\n");
    exit(1);
  }
    
  shift_flags();
  
  if (strcmp(flag, "--version") == 0) {
    print_version();
    return 0;
  } else if (strcmp(flag, "--help") == 0) {
    system("man rm");
    return 0;
  } else if (strcmp(flag, "-v") == 0) {
    verbose = true;
    shift_flags();
  } else if(strcmp(flag, "-i") == 0) {
    prompt_every = true;
    shift_flags();    
  } else if(strcmp(flag, "-d") == 0) {
    remove_dir = true;
    shift_flags();     
  } else {
    filename = flag;
  }

  while (argc >= 0) {
    if(prompt_every) {
      printf("remove file `%s`? ", filename);
      char prompt[16] = {0};
      int err = scanf("%s", prompt);
      if(err == EOF) {
        fprintf(stderr, "error: EOF\n");
        exit(1);
      }
      if(strncmp(prompt, "y", sizeof("y")-1) != 0) {
        filename = *shift(&argc, &argv);
        continue;
      }
    }
    rm(filename);
    filename = *shift(&argc, &argv);
  }
  return 0;
}
