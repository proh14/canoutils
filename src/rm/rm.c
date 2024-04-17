#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define NAME "rm (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "CobbCoding"

bool verbose = false;
bool prompt_every = false;
bool remove_dir = false;
bool force = false;
bool intrusive = false;
bool preserve_root = true;
bool recurse = false;

char **shift(int *argc, char ***argv);
void remove_recursively(DIR *dir, size_t depth);
void rm_dir(char *filename);
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

#define shift_flags()                                                          \
  do {                                                                         \
    if (argc == 0) {                                                           \
      print_not_enough();                                                      \
    }                                                                          \
    flag = *shift(&argc, &argv);                                               \
    filename = flag;                                                           \
  } while (0)

char **shift(int *argc, char ***argv) {
  char **result = *argv;
  *argv += 1;
  *argc -= 1;
  return result;
}

#define DT_DIR 4
#define DT_REG 8

void rm_dir(char *filename) {
    int err = remove(filename);
    if (err == -1 && !force) {
      fprintf(stderr, "could not remove file `%s`\n", filename);
      exit(1);
    }

    if (verbose) {
      printf("removing `%s`\n", filename);
    }
}
	
void remove_recursively(DIR *dir, size_t depth) {
    assert(dir);
	struct dirent *file = readdir(dir);
	while(file != NULL) {
		if(file->d_type == DT_REG) {
			rm(file->d_name);
		} else if(file->d_type == DT_DIR) {
            if(strncmp(file->d_name, ".", 1) == 0) {
              file = readdir(dir);
              continue;
            }
			DIR *new_dir = opendir(file->d_name);
            if(chdir(file->d_name) == -1) {
              fprintf(stderr, "couldn't change directories %d\n", errno);
              exit(1);
            }
			remove_recursively(new_dir, depth+1);
            for(size_t i = 0; i <= depth; i++) {
              if(chdir("..") == -1) {
                fprintf(stderr, "could not change directories %d\n", errno);
                exit(1);
              }
            }
			closedir(new_dir);
			rm_dir(file->d_name);
		} else {
          fprintf(stderr, "error\n");
          exit(1);
        }
        file = readdir(dir);
	}
}

int rm(char *filename) {
  assert(filename != NULL);
  DIR *is_dir = opendir(filename);
  if (!remove_dir && is_dir != NULL && !recurse) {
    fprintf(stderr, "`%s` is a directory\n", filename);
    if (!force)
      goto rm_defer;
  }

  if(is_dir != NULL && recurse) {
    if(chdir(filename) == -1) {
      fprintf(stderr, "could not change directories\n");
      exit(1);
    }
    remove_recursively(opendir("."), 0);
    if(chdir("..") == -1) {
      fprintf(stderr, "could not change directories\n");
      exit(1);
    }
    rm_dir(filename);
    goto rm_end;
  }

  if (!is_dir || remove_dir) {
    int err = remove(filename);
    if (err == -1 && !force) {
      fprintf(stderr, "could not remove file `%s`\n", filename);
      goto rm_defer;
    }

    if (verbose) {
      printf("removing `%s`\n", filename);
    }
  }
  if (is_dir != NULL && closedir(is_dir) == -1) {
    fprintf(stderr, "could not close dir %s\n", filename);
  }

rm_end:
  return 0;

rm_defer:
  if (is_dir != NULL) {
    if (closedir(is_dir) == -1) {
      fprintf(stderr, "could not close dir %s\n", filename);
    }
    exit(1);
  }
  return 1;
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

  // TODO: stacking flags (i.e. rm -rf)
  if (strcmp(flag, "--version") == 0) {
    print_version();
    return 0;
  } else if (strcmp(flag, "--help") == 0) {
    system("man rm");
    return 0;
  } else if (strcmp(flag, "-v") == 0) {
    verbose = true;
    shift_flags();
  } else if (strcmp(flag, "-i") == 0) {
    prompt_every = true;
    shift_flags();
  } else if (strcmp(flag, "-I") == 0) {
    intrusive = true;
    shift_flags();
  } else if (strcmp(flag, "-d") == 0) {
    remove_dir = true;
    shift_flags();
  } else if (strcmp(flag, "-f") == 0) {
    force = true;
    shift_flags();
  } else if (strcmp(flag, "-r") == 0) {
    recurse = true;
    shift_flags();
  } else if (strcmp(flag, "--no-preserve-root") == 0) {
	preserve_root = false;
    shift_flags();
  } else if (strcmp(flag, "--preserve-root") == 0) {
	preserve_root = true;
    shift_flags();
  } else if (strcmp(flag, "--one-file-system") == 0) {
    assert(false && "not implemented yet");
    shift_flags();
  } else if (strcmp(flag, "--interactive") == 0) {
    assert(false && "not implemented yet");
    shift_flags();
  } else {
    filename = flag;
  }

  if (argc > 2 && intrusive) {
    printf("remove %d files? ", argc + 1);
    char prompt[16] = {0};
    char *err = fgets(prompt, 16, stdin);
    if (err == NULL) {
      fprintf(stderr, "error: EOF\n");
      exit(1);
    }
    if (strncmp(prompt, "y", sizeof("y")) != 0)
      exit(0);
  }

  while (argc >= 0) {
	if(strcmp(filename, "/") == 0) {
		printf("cannot remove root directory (see --no-preserve-root)\n");
		filename = *shift(&argc, &argv);
		continue;
	}
    if (prompt_every) {
      printf("remove file `%s`? ", filename);
      char prompt[16] = {0};
      char *err = fgets(prompt, 16, stdin);
      if (err == NULL) {
        fprintf(stderr, "error: EOF\n");
        exit(1);
      }
      if (strncmp(prompt, "y", sizeof("y") - 1) != 0) {
        filename = *shift(&argc, &argv);
        continue;
      }
    }

    if (strcmp(filename, "/") == 0) {
      fprintf(stderr, "cannot remove root directory. See --no-preserve-root\n");
      exit(1);
    }
    rm(filename);
    filename = *shift(&argc, &argv);
  }
  return 0;
}
