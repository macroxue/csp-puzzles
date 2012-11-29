#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "Different.h"
#include "FunctionAC.h"
#include "Problem.h"

class Kakuro : public Problem<int>
{
    public:
        Kakuro(Option option);
        void ShowSolution();

    private:
        static const int M = 30;
        Variable<int>  *v[M][M];

        int down_sum[M][M];
        int right_sum[M][M];

        int rows, cols;

        struct Add
        {
            int operator() (int x, int y) const { return x + y; }
        };

        struct Sub
        {
            int operator() (int x, int y) const { return x - y; }
        };
};

Kakuro::Kakuro(Option option)
    : Problem<int>(option)
{
    // Variables 
    rows = 0;
    char line[M*8];
    while (!feof(stdin) && fgets(line, sizeof(line), stdin)) {
        // Read input line
        int x = 0;
        char *ptr = line;
        while (*ptr) {
            while (*ptr && isspace(*ptr)) ptr++;
            if (*ptr) {
                v[x][rows] = NULL;
                down_sum[x][rows] = right_sum[x][rows] = 0;
                if (*ptr == '?')
                    v[x][rows] = new Variable<int>(1, 9);
                else if (isdigit(*ptr))
                    sscanf(ptr, "%d\\%d", &down_sum[x][rows], &right_sum[x][rows]);
                else if (*ptr == '\\')
                    sscanf(ptr, "\\%d", &right_sum[x][rows]);
                else {
                    printf("Wrong input line #%d: %s", rows+1, line);
                    exit(-1);
                }
                if (x > 0 && v[x-1][rows] == NULL && v[x][rows] == NULL)
                    right_sum[x-1][rows] = 0;
                x++;
            }
            while (*ptr && !isspace(*ptr)) ptr++;
        }

        // Break if nothing on the line
        if (x == 0)
            break;
        cols = x;
        rows++;
    }

    // Constraints on rows
    Different<int> *c = NULL;
    FunctionAC<int, Add, Sub> *s = NULL; 
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (right_sum[x][y] > 0) {
                if (c) AddConstraint(c);
                c = new Different<int>();

                if (s) AddConstraint(s);
                s = new FunctionAC<int, Add, Sub>(right_sum[x][y]);
            }
            if (v[x][y]) {
                c->AddVariable(v[x][y]);
                s->AddVariable(v[x][y]);
            }
        }
    }
    if (c) AddConstraint(c);
    if (s) AddConstraint(s);

    // Constraints on columns
    c = NULL;
    s = NULL;
    for (int x = 0; x < cols; x++) {
        for (int y = 0; y < rows; y++) {
            if (down_sum[x][y] > 0) {
                if (c) AddConstraint(c);
                c = new Different<int>();

                if (s) AddConstraint(s);
                s = new FunctionAC<int, Add, Sub>(down_sum[x][y]);
            }
            if (v[x][y]) {
                c->AddVariable(v[x][y]);
                s->AddVariable(v[x][y]);
            }
        }
    }
    if (c) AddConstraint(c);
    if (s) AddConstraint(s);
}

void Kakuro::ShowSolution()
{
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (v[x][y]) {
                int value = v[x][y]->GetValue(0);
                printf("  %d   ", value);
            } else {
                if (down_sum[x][y] > 0)
                    printf("%2d\\", down_sum[x][y]);
                else
                    printf("  \\");
                if (right_sum[x][y] > 0)
                    printf("%d%s ", right_sum[x][y], right_sum[x][y] < 10 ? " " : "");
                else
                    printf("   ");
            }
        }
        putchar('\n');
    }
}

int main(int argc, char *argv[])
{
    Option option;
    option.sort = Option::SORT_DOMAIN_SIZE;
    option.GetOptions(argc, argv);

    Kakuro puzzle(option);
    puzzle.Solve();

    return 0;
}

