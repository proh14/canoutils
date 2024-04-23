#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define NAME "rmdir (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "tim-tm"

#include "version_info.h"

bool ignore_fail = false;
bool parents = false;
bool verbose = false;

int rm_dir(char *dirname) {
  if (dirname == NULL) {
    fprintf(stderr, "Directory name must be specified.\n");
    return 1;
  }

  struct stat stat_path;
  stat(dirname, &stat_path);
  if (S_ISDIR(stat_path.st_mode) == 0) {
    fprintf(stderr, "'%s' is not a directory.\n", dirname);
    return 1;
  }

  DIR *dir = opendir(dirname);
  if (dir == NULL) {
    fprintf(stderr, "Failed to open '%s': %s\n", dirname, strerror(errno));
    return 1;
  }

  int n = 0;
  struct dirent *dent;
  while ((dent = readdir(dir)) != NULL) {
    if (++n > 2)
      break;
  }
  closedir(dir);

  if (n <= 2) { // Directory is empty (the two entries are '.' and '..')
    if (remove(dirname) != 0) {
      fprintf(stderr, "Failed to remove '%s': %s\n", dirname, strerror(errno));
      return 1;
    } else if (verbose) {
      printf("Removing '%s'\n", dirname);
    }
  } else if (!ignore_fail) {
    fprintf(stderr, "'%s' must be empty.\n", dirname);
    return 1;
  }
  return 0;
}

void strip_off_slash(char *str) {
  if (str == NULL)
    return;

  size_t str_len = strlen(str);
  if (str[str_len - 1] == '/') {
    str[str_len - 1] = '\0';
  }
}

int main(int argc, char **argv) {
  if (argc <= 1) {
    fprintf(stderr,
            "Not enough arguments.\nSee rmdir --help for more information.\n");
    return 1;
  }

  if (strcmp(argv[1], "--version") == 0) {
    print_version();
    return 0;
  }
  if (strcmp(argv[1], "--help") == 0) {
    system("man rmdir");
    return 0;
  }

  if (argc == 2) {
    return rm_dir(argv[1]);
  }

  for (int i = 1; i < argc - 1; ++i) {
    if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--parents") == 0) {
      parents = true;
    } else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--verbose") == 0) {
      verbose = true;
    } else if (strcmp(argv[i], "--ignore-fail-on-non-empty") == 0) {
      ignore_fail = true;
    }
  }

  char *str = argv[argc - 1];
  if (parents) {
    strip_off_slash(str);

    long i = -1;
    while (i != 0) {
      strip_off_slash(str);
      char *strip = strrchr(str, '/');
      if (strip == NULL) {
        i = 0;
      } else {
        i = strip - str + 1;
      }

      if (rm_dir(str) != 0)
        return 1;
      str[i] = '\0';
    }
    return 0;
  } else {
    return rm_dir(str);
  }
}
