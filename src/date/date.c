#define _POSIX_C_SOURCE 200112L // for setenv

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NAME "date (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Eike Flath"

#include "version_info.h"

#define INITIAL_BUF_SIZE 512

static struct tm *get_time(void) {
  time_t t;
  if (time(&t) == -1) {
    fprintf(stderr, "date: failed to retrieve time: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return localtime(&t);
}

static int set_time(char *str) {
  size_t n;
  for (n = 0; str[n]; n++) {
    if (!isdigit(str[n])) {
      fprintf(stderr, "date: invalid date '%s'\n", str);
      return EXIT_FAILURE;
    }
  }

#define s(i) (str[i] - '0')
  struct tm tm;
  memcpy(&tm, get_time(), sizeof tm);
  bool century_specified = false;
  switch (n) {
  case 12:
    tm.tm_year = 100 * (10 * s(n - 4) + s(n - 3));
    century_specified = true;
    // fallthrough
  case 10:
    if (century_specified) {
      tm.tm_year += 10 * s(n - 2) + s(n - 1);
    } else {
      int yy = 10 * s(n - 2) + s(n - 1);
      int cc = yy >= 69 ? 19 : 20;
      tm.tm_year = 100 * cc + yy;
    }
    tm.tm_year -= 1900;
    // fallthrough
  case 8:
    tm.tm_mon = 10 * s(0) + s(1) - 1; // tm.tm_mon is in [0,11]
    tm.tm_mday = 10 * s(2) + s(3);
    tm.tm_hour = 10 * s(4) + s(5);
    tm.tm_min = 10 * s(6) + s(7);
    tm.tm_isdst = -1;
    break;
  default:
    fprintf(stderr, "date: invalid date '%s'\n", str);
    return EXIT_FAILURE;
  }
#undef s
  time_t t = mktime(&tm);
  if (t == -1) {
    fprintf(stderr, "date: '%s' does not specify a valid time\n", str);
    return EXIT_FAILURE;
  }
  struct timespec ts = {.tv_sec = t};
  if (clock_settime(CLOCK_REALTIME, &ts)) {
    fprintf(stderr, "date: cannot set date: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  char *operand = "+%a %b %e %H:%M:%S %Z %Y";
  bool parse_options = true;
  for (int i = 1; i < argc; i++) {
    if (parse_options) {
      if (!strcmp(argv[i], "--")) {
        parse_options = false;
        continue;
      }
      if (!strcmp(argv[i], "--version")) {
        print_version();
        return EXIT_SUCCESS;
      }
      if (!strcmp(argv[i], "-u")) {
        if (setenv("TZ", "UTC", 1) == -1) {
          fprintf(stderr, "date: cannot setenv TZ: %s\n", strerror(errno));
          return EXIT_FAILURE;
        }
        continue;
      }
    }
    operand = argv[i];
  }

  if (operand[0] != '+')
    return set_time(operand);

  struct tm *tm = get_time();

  size_t cap = INITIAL_BUF_SIZE;
  char *buf = malloc(cap);
  if (!buf) {
    fprintf(stderr, "date: malloc failed: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  while (!strftime(buf, cap, &operand[1], tm)) {
    cap *= 2;
    char *new_buf = realloc(buf, cap);
    if (!new_buf) {
      free(buf);
      fprintf(stderr, "date: realloc failed: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }
    buf = new_buf;
  }
  printf("%s\n", buf);
  free(buf);
  return EXIT_SUCCESS;
}
