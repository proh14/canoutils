#include "cgetopt.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#define NAME "mkdir (canoutils)"
#define AUTHOR "Hoorad Farrokh (proh14)"
#define VERSION "1.0.0"

#include "version_info.h"

static const struct option longopts[] = {
    {"help", no_argument, NULL, -1},        {"version", no_argument, NULL, -2},
    {"parent", no_argument, NULL, 'p'},     {"verbose", no_argument, NULL, 'v'},
    {"mode", required_argument, NULL, 'm'}, {NULL, 0, NULL, 0}};

static mode_t mode = 0777;
static int pflag = 0;
static int vflag = 0;

int str_to_mode(const char *str, mode_t *mode) {
  char *endptr = NULL;
  *mode = (mode_t)strtol(str, &endptr, 8);
  if (!endptr)
    return 0;
  while (isspace(*endptr))
    endptr++;
  return *endptr == '\0' && (int)*mode < 010000;
}

static void make_dir(const char *path) {
  if (mkdir(path, mode) == -1) {
    if (!pflag) {
      fprintf(stderr, "mkdir: cannot create directory '%s': %s \n", path,
              strerror(errno));
    }
    return;
  }
  if (vflag)
    printf("mkdir: created directory '%s'\n", path);
}

static void pmake_dir(char *path) {
  char *slash = path;
  for (;;) {
    slash += strspn(slash, "/");
    slash += strcspn(slash, "/");
    int exit = *slash == '\0';
    *slash = '\0';
    make_dir(path);
    *slash = '/';
    if (exit)
      return;
  }
}

static void print_usage(void) {
  printf("Usage: mkdir [OPTION]... DIRECTORY...\n");
}

int main(int argc, char **argv) {
  int ch;

  while ((ch = getopt_long(argc, argv, "pvm:", longopts, NULL)) != -1) {
    switch (ch) {
    case 'p':
      pflag = 1;
      break;
    case 'v':
      vflag = 1;
      break;
    case 'm':
      if (!str_to_mode(optarg, &mode)) {
        fprintf(stderr, "Invalid mode: %s\n", optarg);
        return EXIT_FAILURE;
      }
      break;
    case -1:
      print_usage();
      break;
    case -2:
      print_version();
      break;
    default:
      print_usage();
      return EXIT_FAILURE;
      break;
    }
  }

  argv += optind;
  argc -= optind;

  for (int i = 0; i < argc; i++) {
    if (pflag)
      pmake_dir(argv[i]);
    else
      make_dir(argv[i]);
  }

  return EXIT_SUCCESS;
}
