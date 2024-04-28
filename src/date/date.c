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

static struct tm *get_time(long *nanos) {
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts)) {
    fprintf(stderr, "date: failed to retrieve time: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (nanos)
    *nanos = ts.tv_nsec;
  return localtime(&ts.tv_sec);
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
  long ns;
  memcpy(&tm, get_time(&ns), sizeof tm);
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
  struct timespec ts = {.tv_sec = t, .tv_nsec = ns};
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
    set_operand("+%FT%T%:z");
  } else if (!strcmp(iso, "ns")) {
    set_operand("+%FT%T,%N%:z");
  } else {
    fprintf(stderr, "date: invalid argument '%s' for '--iso-8601'\n", iso);
    fprintf(stderr,
            "must be one of 'hours', 'minutes', 'date', 'seconds', 'ns'\n");
    exit(EXIT_FAILURE);
  }
}

static void use_rfc3339_fmt(char *rfc) {
  if (!strcmp(rfc, "date")) {
    set_operand("+%F");
  } else if (!strcmp(rfc, "seconds")) {
    set_operand("+%F %T%:z");
  } else if (!strcmp(rfc, "ns")) {
    set_operand("+%F %T.%N%:z");
  } else {
    fprintf(stderr, "date: invalid argument '%s' for '--rfc-3339'\n", rfc);
    fprintf(stderr, "must be one of 'date', 'seconds', 'ns'\n");
    exit(EXIT_FAILURE);
  }
}

static void use_rfc_mail_fmt(void) { set_operand("+%a, %d %b %Y %T %z"); }

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
  case 'R':
    use_rfc_mail_fmt();
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
  if (!strcmp(opt, "rfc-email")) {
    use_rfc_mail_fmt();
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
  if (strpre("rfc-3339", opt)) {
    char *arg = opt + strlen("rfc-3339");
    if (*arg != '=')
      goto unknown_opt;
    use_rfc3339_fmt(arg + 1);
    return;
  }
unknown_opt:
  fprintf(stderr, "date: unknown option '--%s'\n", opt);
  exit(EXIT_FAILURE);
}

/**
 * Works just like strftime, except it recognizes formats like %N or %:z which
 * are specified by the GNU date utility but not by the glibc strftime
 * implementation.
 *
 * Currently missing: %::z, %:::z
 */
static size_t extended_strftime(char *restrict buf, size_t max,
                                const char *restrict fmt,
                                const struct tm *restrict tm, long nanos) {
  size_t fmt_size = strlen(fmt) + 1;
  char *f = malloc(fmt_size);
  if (!f) {
    fprintf(stderr, "date: malloc failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  // copy the fmt string into buffer so that we can mutate it
  memcpy(f, fmt, fmt_size);
  size_t n = 0;
  char c;
  int i, j = 0;
  for (i = 0; (c = f[i]); i++) {
    if (c != '%')
      continue;
    switch (f[i + 1]) {
    case '%':
      i++; // skip additional %
      continue;
    case ':':
      if (f[i + 2] != 'z')
        continue;
      // fallthrough
    case 'N':
      if (i > j) {
        // evaluate the "normal" format string until i using strftime
        f[i] = '\0'; // tells strftime to stop at i
        size_t m = strftime(buf, max - n, f + j, tm);
        if (m == 0) {
          free(f);
          return 0;
        }
        n += m;
        buf += m;
      }
      break;
    default:
      continue;
    }
    switch (f[i + 1]) {
    case ':':            // means %:z
      if (max - n < 7) { // need at least 7 bytes: +hh:mm\0
        free(f);
        return 0;
      }
      strftime(buf, 6, "%z", tm); // write +hhmm\0
      memmove(buf + 4, buf + 3, 2);
      buf[3] = ':';
      n += 6;
      buf += 6;
      j = i + 3;
      break;
    case 'N':
      if (max - n < 10) {
        free(f);
        return 0;
      }
      snprintf(buf, 10, "%09ld", nanos);
      n += 9;
      buf += 9;
      j = i + 2;
      break;
    default:
      __builtin_unreachable();
    }
    i = j - 1;
  }
  if (i > j) {
    // evaluate the "normal" format string until i using strftime
    size_t m = strftime(buf, max - n, f + j, tm);
    if (m == 0) {
      free(f);
      return 0;
    }
    n += m;
    buf += m;
  }
  *buf = '\0';
  free(f);
  return n;
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

  long ns;
  struct tm *tm = get_time(&ns);

  size_t cap = INITIAL_BUF_SIZE;
  char *buf = malloc(cap);
  if (!buf) {
    fprintf(stderr, "date: malloc failed: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  while (!extended_strftime(buf, cap, operand + 1, tm, ns)) {
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
