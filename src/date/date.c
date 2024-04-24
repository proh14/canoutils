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

static char *operand;

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

static void set_operand(char *o) {
  if (operand) {
    fprintf(stderr, "date: multiple output formats or dates specified\n");
    exit(EXIT_FAILURE);
  }
  operand = o;
}

static void use_utc(void) {
  if (setenv("TZ", "UTC", 1) == -1) {
    fprintf(stderr, "date: cannot setenv TZ: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

static void use_iso_fmt(char *iso) {
  if (!strcmp(iso, "hours")) {
    set_operand("+%FT%H%:z");
  } else if (!strcmp(iso, "minutes")) {
    set_operand("+%FT%H:%M%:z");
  } else if (!strcmp(iso, "date")) {
    set_operand("+%F");
  } else if (!strcmp(iso, "seconds")) {
    set_operand("+%FT%H:%M:%S%:z");
  } else if (!strcmp(iso, "ns")) {
    set_operand("+%FT%H:%M:%S,%N%:z");
  } else {
    fprintf(stderr, "date: invalid argument '%s' for '--iso-8601'\n", iso);
    fprintf(stderr,
            "must be one of 'hours', 'minutes', 'date', 'seconds', 'ns'\n");
    exit(EXIT_FAILURE);
  }
}

static void short_opt(char *opt) {
  switch (*opt) {
  case 'u':
    use_utc();
    break;
  case 'I':
    if (opt[1])
      use_iso_fmt(opt + 1);
    else
      use_iso_fmt("date");
    break;
  default:
    fprintf(stderr, "date: unknown option '-%c'\n", *opt);
    exit(EXIT_FAILURE);
  }
}

static bool strpre(const char *pre, const char *str) {
  return !strncmp(pre, str, strlen(pre));
}

static void long_opt(char *opt) {
  if (!strcmp(opt, "version")) {
    print_version();
    exit(EXIT_SUCCESS);
  }
  if (!strcmp(opt, "help"))
    exit(system("man date"));
  if (!strcmp(opt, "utc") || !strcmp(opt, "universal")) {
    use_utc();
    return;
  }
  if (strpre("iso-8601", opt)) {
    char *arg = opt + strlen("iso-8601");
    if (*arg) {
      if (*arg != '=')
        goto unknown_opt;
      use_iso_fmt(arg + 1);
    } else {
      use_iso_fmt("date");
    }
    return;
  }
unknown_opt:
  fprintf(stderr, "date: unknown option '--%s'\n", opt);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  operand = NULL;
  bool parse_options = true;
  for (int i = 1; i < argc; i++) {
    if (parse_options && argv[i][0] == '-') {
      if (argv[i][1] == '-') {
        char *opt = argv[i] + 2;
        if (!*opt)
          parse_options = false;
        else
          long_opt(opt);
        continue;
      } else {
        if (argv[i][1]) {
          short_opt(argv[i] + 1);
          continue;
        }
      }
    }
    set_operand(argv[i]);
  }
  if (!operand)
    operand = "+%a %b %e %H:%M:%S %Z %Y";

  if (operand[0] != '+')
    return set_time(operand);

  struct tm *tm = get_time();

  size_t cap = INITIAL_BUF_SIZE;
  char *buf = malloc(cap);
  if (!buf) {
    fprintf(stderr, "date: malloc failed: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  while (!strftime(buf, cap, operand + 1, tm)) {
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
