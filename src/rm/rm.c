#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <unistd.h>

#define NAME "rm (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "CobbCoding"

#include "cgetopt.h"
#include "version_info.h"

struct statvfs cur_filesystem = {0};

char **shift(int *argc, char ***argv);
void remove_recursively(DIR *dir, int flags);
void rm_dir(char *filename, int flags);
int rm(char *filename, int flags);
void handle_remove(int *argc, char ***argv, int flags, char *filename);
void set_flags(int *flags, int *argc, char ***argv);
enum {
  F_HELP = -2,
  F_VERSION = -3,
  F_NO_PRESERVE = -4,
  F_PRESERVE = -5,
  F_ONE_FILESYSTEM = -6,
};

static const struct option longopts[] = {
    {"version", no_argument, NULL, F_VERSION},
    {"help", no_argument, NULL, F_HELP},
    {"no-preserve-root", no_argument, NULL, F_NO_PRESERVE},
    {"preserve-root", no_argument, NULL, F_PRESERVE},
    {"one-file-system", no_argument, NULL, F_ONE_FILESYSTEM},
    {"verbose", no_argument, NULL, 'v'},
    {"recursive", no_argument, NULL, 'r'},
    {"dir", no_argument, NULL, 'd'},
    {NULL, 0, NULL, 0}};

enum {
  F_FORCE = 1 << 0,
  F_PROMPT = 1 << 1,
  F_INTRUSIVE = 1 << 2,
  F_OFS = 1 << 3,
  F_NPR = 1 << 4,
  F_PR = 1 << 5,
  F_RECURSIVE = 1 << 6,
  F_DIR = 1 << 7,
  F_VERBOSE = 1 << 8,
};

char **shift(int *argc, char ***argv) {
  char **result = *argv;
  *argv += 1;
  *argc -= 1;
  return result;
}

#define DT_DIR 4
#define DT_REG 8

void rm_dir(char *filename, int flags) {
  int err = remove(filename);
  if (err == -1 && !(flags & F_FORCE)) {
    fprintf(stderr, "could not remove dir `%s`\n", filename);
    exit(1);
  }

  if (flags & F_VERBOSE) {
    printf("removing `%s`\n", filename);
  }
}

void remove_recursively(DIR *dir, int flags) {
  assert(dir);
  struct dirent *file = readdir(dir);
  while (file != NULL) {
    if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
      file = readdir(dir);
      continue;
    }
    if (flags & F_OFS) {
      struct statvfs new_filesystem = {0};
      if (statvfs(file->d_name, &new_filesystem) != 0) {
        fprintf(stderr, "could not get filesystem\n");
        exit(1);
      }
      if (cur_filesystem.f_fsid != new_filesystem.f_fsid) {
        fprintf(stderr, "file `%s` is on a different filesystem, skipping\n",
                file->d_name);
        file = readdir(dir);
        continue;
      }
    }
    if (file->d_type == DT_REG) {
      rm(file->d_name, flags);
    } else if (file->d_type == DT_DIR) {
      DIR *new_dir = opendir(file->d_name);
      if (chdir(file->d_name) == -1) {
        fprintf(stderr, "couldn't change directories %d\n", errno);
        exit(1);
      }
      remove_recursively(new_dir, flags);
      if (chdir("..") == -1) {
        fprintf(stderr, "could not change directories %d\n", errno);
        exit(1);
      }
      closedir(new_dir);
      rm_dir(file->d_name, flags);
    } else {
      fprintf(stderr, "error\n");
      exit(1);
    }
    file = readdir(dir);
  }
}

int rm(char *filename, int flags) {
  assert(filename != NULL);
  DIR *is_dir = opendir(filename);
  if (!(flags & F_DIR) && is_dir != NULL && !(flags & F_RECURSIVE)) {
    fprintf(stderr, "`%s` is a directory\n", filename);
    if (!(flags & F_FORCE))
      goto rm_defer;
  }

  if (is_dir != NULL && (flags & F_RECURSIVE)) {
    if (chdir(filename) == -1) {
      fprintf(stderr, "could not change directories\n");
      exit(1);
    }
    remove_recursively(opendir("."), flags);
    if (chdir("..") == -1) {
      fprintf(stderr, "could not change directories\n");
      exit(1);
    }
    rm_dir(filename, flags);
    goto rm_end;
  }

  if (!is_dir || (flags & F_DIR)) {
    int err = remove(filename);
    if (err == -1 && !(flags & F_FORCE)) {
      fprintf(stderr, "could not remove file `%s`\n", filename);
      goto rm_defer;
    }

    if (flags & F_VERBOSE) {
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

void handle_remove(int *argc, char ***argv, int flags, char *filename) {
    if (strcmp(filename, "/") == 0 && !(flags & F_NPR)) {
      printf("cannot remove root directory (see --no-preserve-root)\n");
      filename = *shift(argc, argv);
      return;
    }
    if (flags & F_PROMPT) {
      printf("remove file `%s`? ", filename);
      char prompt[16] = {0};
      char *err = fgets(prompt, 16, stdin);
      if (err == NULL) {
        fprintf(stderr, "error: EOF\n");
        exit(1);
      }
      if (strncmp(prompt, "y", 1) != 0) {
        filename = *shift(argc, argv);
        return;
      }
    }
    rm(filename, flags);
}

void set_flags(int *flags, int *argc, char ***argv) {
  int c = getopt_long(*argc, *argv, "viIdfr", longopts, NULL);
  while (c != -1) {
    switch (c) {
    case F_VERSION:
      print_version();
      exit(0);
    case F_HELP:
      if (system("man rm")) {
        fprintf(stderr, "error: please install man to see help page\n");
        exit(1);
      };
      break;
    case 'v':
      *flags |= F_VERBOSE;
      break;
    case 'i':
      *flags &= ~(F_FORCE);
      *flags |= F_PROMPT;
      break;
    case 'I':
      *flags &= ~(F_FORCE);
      *flags |= F_INTRUSIVE;
      break;
    case 'd':
      *flags |= F_DIR;
      break;
    case 'f':
      *flags &= ~(F_PROMPT);
      *flags &= ~(F_INTRUSIVE);
      *flags |= F_FORCE;
      break;
    case 'R':
    case 'r':
      *flags |= F_RECURSIVE;
      break;
    case F_NO_PRESERVE:
      *flags &= ~(F_NPR);
      *flags |= F_PR;
      break;
    case F_PRESERVE:
      *flags &= ~(F_PR);
      *flags |= F_NPR;
      break;
    case F_ONE_FILESYSTEM:
      *flags |= F_RECURSIVE;
      *flags |= F_OFS;
      if (statvfs(".", &cur_filesystem) != 0) {
        fprintf(stderr, "could not get filesystem\n");
        exit(1);
      }
      break;
    default:
      fprintf(stderr, "unknown flag\n");
      exit(1);
      break;
    }
    c = getopt_long(*argc, *argv, "viIdfr", longopts, NULL);
  }

  if (optind > *argc) {
    fprintf(stderr, "error: filename not provided\n");
    exit(1);
  }

  *argv += optind;
  *argc -= optind;

  if(*argc > 3 && (*flags & F_INTRUSIVE)) *flags |= F_PROMPT;
}

int main(int argc, char **argv) {
  char *filename = NULL;
  if (argc < 2) {
    fprintf(stderr, "not enough args\n");
    fprintf(stderr, "see rm --help\n");
    exit(1);
  }

  int flags = 0;

  set_flags(&flags, &argc, &argv);

  filename = *shift(&argc, &argv);

  if (argc > 2 && (flags & F_RECURSIVE)) {
    printf("remove %d files? ", argc + 1);
    char prompt[16] = {0};
    char *err = fgets(prompt, 16, stdin);
    if (err == NULL) {
      fprintf(stderr, "error: EOF\n");
      exit(1);
    }
    if (strncmp(prompt, "y", 1) != 0)
      exit(0);
  }

  while(argc >= 0) {
    handle_remove(&argc, &argv, flags, filename);
    filename = *shift(&argc, &argv);
  }
  return 0;
}