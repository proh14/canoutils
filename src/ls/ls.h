#ifndef MY_LS_H
#define MY_LS_H

#include <stdbool.h>
#include <stddef.h>

#define ZERO_OR(expr, default) ((!!(expr)) * default)

#define __USE_XOPEN2K8
#define __USE_MISC

#include <linux/limits.h>
#include <sys/stat.h>

#define MIN_ALLOCATED_ENTRY (1024)

#if !defined(__clang__) || !defined(__GNUC__)
#define __attribute__(x) // omitted
#endif

enum {
  F_ALL_FILES = 1 << 0,
  F_LONG_FORM = 1 << 1,
  F_RECURSIVE = 1 << 2,
  F_DIRECTORY = 1 << 3,
  F_REV_ORDER = 1 << 4,
  F_SORT_TIME = 1 << 5,
  F_SHOW_DIRS = 1 << 6,
};

typedef struct {
  struct stat stat;
  struct passwd *passwd;
  struct group *group;
  char name[NAME_MAX + 1];
} entry_t;

typedef struct {
  char *name;
  entry_t *entries;
  size_t size;
  bool is_file;
} dirbuff_t;

inline __attribute__((const)) int stridx(const char *str, char c) {
  for (const char *p = str; *p != '\0'; p++)
    if (*p == c)
      return (int)(p - str);
  return -1;
}

char *strdup(char const *s);

int list_dir(dirbuff_t *db, char flags);
size_t recurse(dirbuff_t *db, size_t count, char flags);

void print_entries(entry_t *entry, size_t count, char flags);
char *path_concat(char *dest, char *basepath, char *suffix);

#endif
