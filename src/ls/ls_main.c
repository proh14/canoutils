#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls.h"

#define NAME "ls (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Yohann Boniface (Sigmanificient)"

#include "version_info.h"

static const char FLAGLIST[] = "alRdrt";
static char DEFAULT_LOCATION[] = ".";

static char compose_flaglist(int argc, char **argv) {
  int flags = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-' || argv[i][1] == '\0')
      continue;
    for (int j = 1; argv[i][j] != '\0'; j++)
      flags |= 1 << (stridx(FLAGLIST, argv[i][j]) + 1);
  }
  return (char)(flags >> 1);
}

static size_t count_targets(int argc, char **argv) {
  int count = 0;

  for (int i = 1; i < argc; i++)
    if (argv[i][0] != '-' || argv[i][1] == '\0')
      count++;
  return count;
}

static bool list_dirs(dirbuff_t *db, int argc, char **argv, char flags) {
  int err = 0;
  size_t count = count_targets(argc, argv);

  if (count == 0) {
    db->name = DEFAULT_LOCATION;
    err |= list_dir(db, flags);
  }
  for (int i = 1; i < argc; i++) {
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
  char flags;
  int err = 0;

  for (int i = 0; argv[i] != NULL; i++)
    if (!strcmp(argv[i], "--version")) {
      print_version();
      return EXIT_SUCCESS;
    }
  flags = compose_flaglist(argc, argv);
  db.entries = malloc(db.size * sizeof(*db.entries));
  if (db.entries == NULL)
    return EXIT_FAILURE;
  if (flags & F_DIRECTORY)
    flags &= ~F_RECURSIVE;
  err |= !list_dirs(&db, argc, argv, flags);
  free(db.entries);
  return err ? EXIT_SUCCESS : EXIT_FAILURE;
}
