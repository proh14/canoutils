#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int ch;
  while ((ch = getopt(argc, argv, "a:b")) != -1) {
    printf("ch = %c\n", ch);
  }
  argv += optind;
  argc -= optind;
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  return 0;
}
