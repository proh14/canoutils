#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define NAME "cksum (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "cgetopt.h"
#include "version_info.h"

static const char usage[] = {
	"Usage: cksum [Option]... [File]...\n"
	"  --version   version information\n"
	"  --help      display this help and exit\n"
	"  --raw       output raw crc\n"
	"  --tag       default output style\n"
};

//flags
static int output_format = 't';

static struct option long_options[] = {
	{"version", no_argument, NULL, 'v'},
	{"help", no_argument, NULL, 'h'},
	{"raw", no_argument, NULL, 'r'},
	{"tag", no_argument, NULL, 't'},
};


static const uint32_t crc32_ = 0x04c11db7;

uint32_t crc32(FILE *file, size_t *octets) {
  uint32_t remainder = 0;
  uint8_t difference = 0;

  size_t length = 0;

  /* calculate crc of file contents */
  int cur_char;
  while ((cur_char = getc(file)) != EOF) {
    remainder = (remainder << 8) ^ (difference & 0xff) ^ cur_char;
    difference = 0;
    for (int i = 31; i > 23; i--) {
      if (remainder & (1 << i)) {
        remainder ^= (1 << i) | (crc32_ >> (32 - i));
        difference ^= crc32_ << (i - 24);
      }
    }
    length++;
  }

  *octets = length;

  /* calculate crc of file contents + length of files in bytes */
  while (length) {
    cur_char = (length & 0xff);
    length >>= 8;

    remainder = (remainder << 8) ^ (difference & 0xff) ^ cur_char;
    difference = 0;
    for (int i = 31; i > 23; i--) {
      if (remainder & (1 << i)) {
        remainder ^= (1 << i) | (crc32_ >> (32 - i));
        difference ^= crc32_ << (i - 24);
      }
    }
  }

  /* work through the remaining 3 bytes  */
  for (int j = 0; j < 3; j++) {
    remainder = (remainder << 8) ^ (difference & 0xff);
    difference = 0;
    for (int i = 31; i > 23; i--) {
      if (remainder & (1 << i)) {
        remainder ^= (1 << i) | (crc32_ >> (32 - i));
        difference ^= crc32_ << (i - 24);
      }
    }
  }

  return ~((remainder << 8) | (difference & 0xff));
}

void crc_print(char *filename, uint32_t crc, size_t octets){
  printf("%c\n", output_format);
  switch(output_format){
    case 't':
      if(filename == NULL)
        printf("%u %lu\n", crc, octets);
      else
        printf("%u %lu %s\n", crc, octets, filename);
      break;
    case 'r':
      printf("%c%c%c%c", (crc >> 24) & 255, (crc >> 16) & 255, (crc >> 8) & 255, crc & 255);
      break;
  }
}

int main(int argc, char **argv) {

  int opt, option_index;
  while ((opt = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
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
    case 'r':
      output_format = 'r';
      break;
    case 't':
      output_format = 't';
      break;
    }
  }

  int index = optind;
  size_t octets;
  uint32_t crc;

  if (argc < 2) {
    crc = crc32(stdin, &octets);
    crc_print(NULL, crc, octets);
    return EXIT_SUCCESS;
  }

  for (; index < argc; index++) {
    FILE *file = fopen(argv[index], "r+");

    if (file == NULL){
      fprintf(stderr, "cksum: %s: %s\n", argv[index], strerror(errno));
      continue;
    }

    crc = crc32(file, &octets);
    crc_print(argv[index], crc, octets);

    fclose(file);
  }
}
