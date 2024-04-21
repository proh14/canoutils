#include <stdio.h>
#include "getopt.h"

int main(int argc, char **argv) {
    struct option opts[] = {
        {
            .name = "log-file",
            .val = 1,
        },
        {0},
    };
    int c;
    while((c = getopt_long(argc, argv, "", opts, NULL)) >= 0) {
		switch(c) {
			case 1:
				printf("log file\n");
				break;
			default:
	        printf("%c\n", c);			
			break;
		}
    }
    argv += optind;
    printf("extra value: %s\n", *argv);
  return 0;
}
