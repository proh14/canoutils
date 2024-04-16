#!/bin/bash

# Define the input file
UTILS=../utils.txt

cd ../src

# Read the input file line by line
while read LINE
do
    if [ -d $LINE ]; then
        echo "----------------------------------------"
        echo "Directory $LINE Already exists; Skipping..."
        continue
    fi
    echo "----------------------------------------"
    mkdir $LINE
    echo "Directory: $LINE created"
    cd $LINE
    printf '#include <stdio.h>\n\nint main(void) {\n  printf("Hello, World!\\n");\n  return 0;\n}' > $LINE.c
    echo "File: $LINE.c created"
    printf '\\" TODO' > $LINE.1
    echo "File: $LINE.1 created"
    printf 'OUT := '$LINE'\n\nSRC := '$LINE'.c\n\ninclude ../../base.mk\ninclude ../../commons.mk\n' > Makefile
    echo "File: Makefile created"
    cd ..
done < "$UTILS"
echo "----------------------------------------"
