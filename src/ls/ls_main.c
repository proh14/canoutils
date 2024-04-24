#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls.h"

#define NAME "ls (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Yohann Boniface (Sigmanificient)"

#include "cgetopt.h"
#include "version_info.h"

static char DEFAULT_LOCATION[] = ".";

enum {
  GETOPT_VERSION_CHAR = (CHAR_MIN - 1),
  GETOPT_SORT_CHAR = (CHAR_MIN - 2),
};

static const struct option LONG_OPTIONS[] = {
    {"version", no_argument, NULL, GETOPT_VERSION_CHAR},
    {"all", no_argument, NULL, 'a'},
    {"recursive", no_argument, NULL, 'R'},
    {"directory", no_argument, NULL, 'd'},
    {"reverse", no_argument, NULL, 'r'},
    {"sort", required_argument, NULL, GETOPT_SORT_CHAR},
    {0}};

static char compose_flaglist(int argc, char **argv) {
  char flags = 0;
  int c;

  while ((c = getopt_long(argc, argv, "alRdrt", LONG_OPTIONS, NULL)) != -1) {
    switch (c) {
    case 'a':
      flags |= F_ALL_FILES;
      break;
    case 'l':
      flags |= F_LONG_FORM;
      break;
    case 'R':
      flags |= F_RECURSIVE;
      break;
    case 'd':
      flags |= F_DIRECTORY;
      flags &= ~F_RECURSIVE;
      break;
    case 'r':
      flags |= F_REV_ORDER;
      break;
    case 't':
      flags |= F_SORT_TIME;
      break;
    case (CHAR_MAX + 1):
      if (!strcmp(optarg, "time"))
        flags |= F_SORT_TIME;
      break;
    case GETOPT_VERSION_CHAR:
      print_version();
      return -1;
    default:
      fprintf(stderr, "ls: invalid option -- '%c'\n", c);
      return -2;
    }
  }
  return flags;
}

static bool list_dirs(dirbuff_t *db, int argc, char **argv, char flags) {
  int err = 0;
  size_t count = argc - optind;

  if (count == 0) {
    db->name = DEFAULT_LOCATION;
    err |= list_dir(db, flags);
  }
  argv += optind;
  argc -= optind;
  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] != '\0')
      continue;
    db->name = argv[i];
    if (count > 1)
      flags |= F_SHOW_DIRS;
    err |= list_dir(db, flags);
  }
  return err;
}

int main(int argc, char **argv) {
  dirbuff_t db = {.size = MIN_ALLOCATED_ENTRY};
  char flags = compose_flaglist(argc, argv);
  int err = 0;

  if (flags < 0)
    return (flags == -1) ? EXIT_SUCCESS : EXIT_FAILURE;
  db.entries = malloc(db.size * sizeof(*db.entries));
  if (db.entries == NULL)
    return EXIT_FAILURE;
  err |= !list_dirs(&db, argc, argv, flags);
  free(db.entries);
  return err ? EXIT_SUCCESS : EXIT_FAILURE;
}
