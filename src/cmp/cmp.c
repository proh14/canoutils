#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "cmp (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "cgetopt.h"
#include "version_info.h"

static const char usage[] = {
	"Usage: cmp [Option]... File1 File2\n"
         "  --version   version information\n"
                             "  --help      display this help and exit\n"
                             "  -l          output byte number (decimal) and differing bytes (octal) for each difference\n"
                             "  -s          no output except exit status\n"};

// flags
static int output_format = 'd';

static struct option long_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
};

static void cmp_print(char *path1, int ch1, char *path2, int ch2, size_t byte, size_t line){
  switch(output_format){
  case 'd':
    printf("%s %s differ: byte %ld, line %ld\n", path1, path2, byte, line);
    break;
  case 'l':
    printf("%ld %o %o\n", byte, ch1, ch2);
    break;
  case 's':
    break; 
  }
}

int main(int argc, char **argv) {
  int opt, option_index;
  while ((opt = getopt_long(argc, argv, "ls", long_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 0:
      break;
    case 'v':
      print_version();
      return EXIT_SUCCESS;
      break;
    case 'h':
      printf("%s\n", usage);
      return EXIT_SUCCESS;
    case 'l':
      output_format = 'l';
      break;
    case 's':
      output_format = 's';
      break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc != 2){
    fprintf(stderr, "cmp: expected two files");
    return EXIT_FAILURE;
  }

  FILE *file1 = fopen(argv[0], "r+");
  if (file1 == NULL){
      fprintf(stderr, "cmp: %s: %s\n", argv[0], strerror(errno));
      return EXIT_FAILURE;
  }

  FILE *file2 = fopen(argv[1], "r+");
  if (file2 == NULL){
      fprintf(stderr, "cmp: %s: %s\n", argv[1], strerror(errno));
      return EXIT_FAILURE;
  }

  int ch1, ch2;
  size_t byte = 1, line = 1;
  char *path = NULL;

  while(1){
    ch1 = getc(file1);
    ch2 = getc(file2);

    if(ch1 == EOF || ch2 == EOF)
      break;

    if(ch1 != ch2){
      cmp_print(argv[0], ch1, argv[1], ch2, byte, line);
      if(output_format != 'l')
	return 1; //files are different
    }

    if(ch1 == '\n')
      line++;
    byte++;
  }

  if(ch1 != EOF || ch2 != EOF){
    if(ch1 == EOF)
      path = argv[0];
    if(ch2 == EOF)
      path = argv[1];
    
    printf("cmp: EOF on %s after byte %lu\n", path, byte);
    return 1; //files are different
  }

  return 0; //files are the same
}
