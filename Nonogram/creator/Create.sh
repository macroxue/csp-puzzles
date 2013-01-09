#!/bin/sh

usage()
{
    cat << USAGE
    Usage: Create.sh <header>

    header - Black and white image exported as C source code header by GIMP
USAGE
    exit 1
}

if [[ $# -lt 1 ]]; then
    usage
fi

NAME=${1%.h}
HEADER=$NAME.h
PROGRAM=$NAME.cpp
if [[ ! -f $HEADER ]]; then
    echo "Image header $HEADER not found"
    exit 1
fi
echo "#include \"$HEADER\"" > $PROGRAM

cat >> $PROGRAM << END
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int factor = 2;
    if (argc > 1)
        factor = atoi(argv[1]);

    int threshold = 1;
    if (argc > 2)
        threshold = atoi(argv[2]);

    char image[width/factor+1][height/factor+1];
    memset(image, 0, sizeof(image));

    char *brown = header_data;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel[4];
            HEADER_PIXEL(brown, pixel);
            image[x/factor][y/factor] += (pixel[0] == 0);
        }
    }

    int rows = height/factor, columns = width/factor;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            putchar(image[x][y] >= threshold ? 'O' : '.');
            putchar(' ');
        }
        putchar('\n');
    }
    
    return 0;
}
END

g++ -o $NAME $PROGRAM
rm $PROGRAM

