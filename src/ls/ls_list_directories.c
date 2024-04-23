#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "ls.h"

static void get_file_info(const char *path, entry_t *entry) {
  if (stat(path, &entry->stat) < 0)
    return;
  entry->passwd = getpwuid(entry->stat.st_uid);
  entry->group = getgrgid(entry->stat.st_gid);
}

static __attribute__((nonnull)) size_t read_directory(dirbuff_t *db, DIR *dir,
                                                      char flags) {
  static char path[PATH_MAX];
  size_t i = 0;

  for (struct dirent *dirent = readdir(dir); dirent != NULL;
       dirent = readdir(dir)) {
    if (dirent->d_name[0] == '.' && ~flags & F_ALL_FILES)
      continue;
    if (i == db->size) {
      db->size <<= 1;
      db->entries = realloc(db->entries, db->size * sizeof(*db->entries));
    }
    strcpy(db->entries[i].name, dirent->d_name);
    if (flags & (F_LONG_FORM | F_SORT_TIME | F_RECURSIVE))
      get_file_info(path_concat(path, db->name, db->entries[i].name),
                    &db->entries[i]);
    i++;
  }
  return i;
}

static void print_error(char *dirname) {
  char const *err = strerror(errno);

  switch (errno) {
  case ENOENT:
    fprintf(stderr, "ls: cannot access '%s': %s\n", dirname, err);
    return;
  case EACCES:
    fprintf(stderr, "ls: cannot open directory '%s': %s\n", dirname, err);
    return;
  default:
    fprintf(stderr, "ls: %s\n", err);
  }
}

static int compare_names(entry_t const *leftp, entry_t const *rightp) {
  return strcoll(leftp->name, rightp->name);
}

static int compare_times(entry_t const *leftp, entry_t const *rightp) {
  return (int)(rightp->stat.st_mtim.tv_sec - leftp->stat.st_mtim.tv_sec);
}

int list_dir(dirbuff_t *db, char flags) {
  struct stat fi;
  size_t count = 1;
  DIR *dir;

  if (stat(db->name, &fi) < 0)
    return print_error(db->name), -1;
  db->is_file = S_ISDIR(fi.st_mode) && ~flags & F_DIRECTORY;
  if (db->is_file) {
    dir = opendir(db->name);
    if (dir == NULL)
      return print_error(db->name), -1;
    count = read_directory(db, dir, flags);
    closedir(dir);
  } else {
    strcpy(db->entries[0].name, db->name);
    get_file_info(db->name, &db->entries[0]);
  }

  qsort(db->entries, count, sizeof *db->entries, (__compar_fn_t)&compare_names);
  if (flags & F_SORT_TIME)
    qsort(db->entries, count, sizeof *db->entries,
          (__compar_fn_t)&compare_times);
  if (flags & (F_SHOW_DIRS | F_RECURSIVE) && !db->is_file)
    printf("%s:\n", db->name);
  print_entries(db->entries, count, flags);
  if (flags & F_RECURSIVE && !db->is_file)
    recurse(db, count, flags);
  return 0;
}
