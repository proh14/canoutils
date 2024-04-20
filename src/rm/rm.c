#include <stdio.h>
#include "./getopt.h"

int main(int argc, char **argv) {
  char c;
  struct option opts[] = {
    {
    .name = "log-file",
    .val = 1,
    },
    {NULL},
  };
  while((c = getopt_long(argc, argv, "abc", opts, NULL)) != -1) {
    printf("%d\n", c);
  }
  argv += optind;
  printf("%s\n", *argv);
  return 0;
}