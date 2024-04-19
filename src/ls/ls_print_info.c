#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/sysmacros.h>
#include <sys/types.h>
#include <time.h>

#include "ls.h"

typedef unsigned char uchar;

static void get_file_right(char bits[static sizeof("rwxrwxrwx")],
                           entry_t *entry) {
  char *bitsp = bits;
  mode_t mode = entry->stat.st_mode;
  static const char *s = "-rwx";

  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IRUSR, 1)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IWUSR, 2)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IXUSR, 3)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IRGRP, 1)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IWGRP, 2)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IXGRP, 3)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IROTH, 1)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IWOTH, 2)];
  *bitsp++ = s[(uchar)ZERO_OR(mode & S_IXOTH, 3)];
  if (mode & S_ISUID)
    bits[1] = (mode & S_IXUSR) ? 's' : 'S';
  if (mode & S_ISGID)
    bits[3] = (mode & S_IXGRP) ? 's' : 'l';
  if (mode & S_ISVTX)
    bits[8] = (mode & S_IXOTH) ? 't' : 'T';
}

static char get_file_type(entry_t *entry) {
  const char typ[] = {
      [S_IFBLK] = 'b', [S_IFCHR] = 'c', [S_IFDIR] = 'd',  [S_IFIFO] = 'p',
      [S_IFLNK] = 'l', [S_IFREG] = '-', [S_IFSOCK] = 's', [0] = '?'};

  return typ[(entry->stat.st_mode & S_IFMT)];
}

static char *get_creation_time(entry_t *entry) {
  static char fmt[sizeof("Jan 01 00:00")];
  char *ct = ctime(&entry->stat.st_mtim.tv_sec);
  time_t now = time(NULL);
  const int six_month_sec = 6 * 24 * 3600 * 31;

  if (strlen(ct) < 24)
    return NULL;
  if (entry->stat.st_mtim.tv_sec + six_month_sec < now) {
    strncpy(fmt, ct + 4, 7);
    strncpy(fmt + 7, ct + 19, 5);
  } else
    strncpy(fmt, ct + 4, 12);
  return fmt;
}

static void print_file_infos(entry_t *entry) {
  struct stat *fi = &entry->stat;
  char perms[sizeof("-rwxr-xr-x")] = {[0] = get_file_type(entry), '\0'};
  char const *owner = (entry->passwd == NULL) ? "?" : entry->passwd->pw_name;
  char const *grp = (entry->group == NULL) ? "?" : entry->group->gr_name;
  char *time = get_creation_time(entry);

  get_file_right(perms + 1, entry);
  printf("%.10s %ld %s %s ", perms, fi->st_nlink, owner, grp);
  if (stridx("bc", perms[0]) != -1)
    printf("%d, %d", major(fi->st_rdev), minor(fi->st_rdev));
  else
    printf("%ld", fi->st_size);
  printf(" %.12s ", time);
}

void print_entries(entry_t *entry, size_t count, char flags) {
  int d;

  if (flags & F_REV_ORDER) {
    d = -1;
    entry += (count - 1);
  } else
    d = 1;
  for (size_t i = 0; i < count; i++) {
    if (flags & F_LONG_FORM)
      print_file_infos(entry);
    printf("%s", entry->name);
    printf(((i + 1) == count || flags & F_LONG_FORM) ? "\n" : "  ");
    entry += d;
  }
}
