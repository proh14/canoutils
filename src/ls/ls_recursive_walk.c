#include <stdlib.h>
#include <string.h>

#include "ls.h"

char *path_concat(char *dest, char *basepath, char *suffix) {
  unsigned long written = 0;

  strcpy(dest, basepath);
  written = strlen(basepath);
  if (dest[written - 1] != '/') {
    dest[written] = '/';
    written++;
  }
  strcpy(dest + written, suffix);
  written += strlen(suffix);
  dest[written] = '\0';
  return dest;
}

static __attribute__((nonnull)) size_t
find_directories(char **dirs, dirbuff_t *db, size_t count) {
  size_t found = 0;

  for (size_t i = 0; i < count; i++) {
    if (!strcmp(db->entries[i].name, ".") || !strcmp(db->entries[i].name, ".."))
      continue;
    if (S_ISDIR(db->entries[i].stat.st_mode)) {
      dirs[found] = strdup(db->entries[i].name);
      found++;
    }
  }
  return found;
}

size_t recurse(dirbuff_t *db, size_t count, char flags) {
  static char path[PATH_MAX];
  size_t dirsize = strlen(db->name);
  char **dirs = malloc(count * sizeof(char *));
  size_t j = find_directories(dirs, db, count);

  for (size_t i = 0; i < j; i++) {
    db->name = path_concat(path, db->name, dirs[i]);
    list_dir(db, flags);
    db->name[dirsize] = '\0';
  }
  for (size_t i = 0; i < j; i++)
    free(dirs[i]);
  free(dirs);
  return 0;
}
