#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#define NAME "cksum (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include "../version_info.h"
#include "../getopt.h"

static const char usage[] = {
	"  -v   version information\n"
};

static const uint32_t crc32_ = 0x04c11db7;

uint32_t crc32(FILE *file, size_t *octets){
	uint32_t remainder = 0;
	uint8_t difference = 0;

	size_t length = 0;


	/* calculate crc of file contents */
	int cur_char;
	while((cur_char = getc(file)) != EOF){
		remainder = (remainder << 8) ^ (difference & 0xff) ^ cur_char;
		difference = 0;
		for(int i = 31; i > 23; i--){
			if(remainder & (1 << i)){
				remainder ^= (1 << i) | (crc32_ >> (32 - i));
				difference ^= crc32_ << (i - 24);
			}
		}
		length++;
	}

	*octets = length;

	/* calculate crc of file contents + length of files in bytes */
	while(length){
		cur_char = (length & 0xff);
		length >>= 8;

		remainder = (remainder << 8) ^ (difference & 0xff) ^ cur_char;
		difference = 0;
		for(int i = 31; i > 23; i--){
			if(remainder & (1 << i)){
				remainder ^= (1 << i) | (crc32_ >> (32 - i));
				difference ^= crc32_ << (i - 24);
			}
		}
	}


	/* work through the remaining 3 bytes  */
	for(int j = 0; j < 3; j++){
		remainder = (remainder << 8) ^ (difference & 0xff);
		difference = 0;
		for(int i = 31; i > 23; i--){
			if(remainder & (1 << i)){
				remainder ^= (1 << i) | (crc32_ >> (32 - i));
				difference ^= crc32_ << (i - 24);
			}
		}
	}

	return ~((remainder<<8)|(difference&0xff));
	
}

int main(int argc, char ** argv) {
	if(argc<2){
		printf("%s\n", usage);
		exit(1);
	}

	int opt;
	while((opt = getopt(argc, argv, "v")) != -1){
		switch(opt){
			case 'v':
				print_version();
				return EXIT_SUCCESS;
			break;
		}
	}

	int index = optind;
	for(;index < argc; index++){
		FILE*file=fopen(argv[index],"r");

		if(file==NULL)
			return EXIT_FAILURE;

		size_t octets;
		uint32_t crc = crc32(file, &octets);

		printf("%u %lu %s\n", crc, octets, argv[index]);
	}
}
