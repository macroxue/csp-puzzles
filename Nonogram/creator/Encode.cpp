#include <ctype.h>
#include <stdio.h>
#include <string.h>

int main()
{
    const int n = 256;
    char image[n][n];
    memset(image, 0, sizeof(image));

    int  rows = 0, columns = 0;
    char line[n*2+1];
    while (!feof(stdin) && fgets(line, sizeof(line), stdin)) {
        int x = 0;
        for (int i = 0; line[i]; i++) {
            if (isspace(line[i]))
                continue;
            else if (line[i] == '.')
                image[x++][rows] = 0;
            else
                image[x++][rows] = 1;
        }
        rows++;
        if (columns < x)
            columns = x;
    }
    
    printf("%d %d\n", rows, columns);
    for (int y = 0; y < rows; y++) {
        int run = 0;
        for (int x = 0; x < columns; x++) {
            if (image[x][y] == 1)
                run++;
            else if (run > 0) {
                printf("%d ", run);
                run = 0;
            }
        }
        if (run > 0)
            printf("%d ", run);
        putchar('\n');
    }
    puts("#");

    for (int x = 0; x < columns; x++) {
        int run = 0;
        for (int y = 0; y < rows; y++) {
            if (image[x][y] == 1)
                run++;
            else if (run > 0) {
                printf("%d ", run);
                run = 0;
            }
        }
        if (run > 0)
            printf("%d ", run);
        putchar('\n');
    }

    return 0;
}
