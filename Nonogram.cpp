#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#include "RunLength.h"
#include "Problem.h"

uint64_t hash_rand[4][128];

class Nonogram : public Problem<bool>
{
    public:
        Nonogram(Option option, bool rootate);
        void ShowState(Variable<bool> *);
        void ShowSolution();

    private:
        template <size_t M, size_t B>
            void CreateRowConstraint(int y);
        template <size_t M, size_t B>
            void CreateColumnConstraint(int x);

        int  columns, rows;
        vector< vector<Variable<bool> * > >  grid;
        vector< vector<int> >  column_runs;
        vector< vector<int> >  row_runs;
};

Nonogram::Nonogram(Option option, bool rotate)
    : Problem<bool>(option)
{
    // Read input
    char line[128];
    fgets(line, sizeof(line), stdin);
    if (!rotate)
        sscanf(line, "%d %d", &rows, &columns);
    else
        sscanf(line, "%d %d", &columns, &rows);

    row_runs.resize(rows);
    column_runs.resize(columns);
    for (int i = 0; i < rows + columns; i++) {
        do fgets(line, sizeof(line), stdin);
        while (line[0] == '#');
        int n[64], count = 0;
        char *ptr = line;
        while (*ptr) {
            while (*ptr && !isdigit(*ptr)) ptr++;
            if (*ptr)
                n[count++] = atoi(ptr);
            while (*ptr && !isspace(*ptr)) ptr++;
        }

        for (int j = 0; j < count; j++) {
            if (!rotate)
                if (i < rows)
                    row_runs[i].push_back(n[j]);
                else
                    column_runs[i-rows].push_back(n[j]);
            else
                if (i < columns)
                    column_runs[i].push_back(n[j]);
                else
                    row_runs[i-columns].push_back(n[j]);
        }
    }

    // Create variables
    bool domain[2] = { false, true };
    grid.resize(columns);
    for (int x = 0; x < columns; x++)
        for (int y = 0; y < rows; y++)
            grid[x].push_back( new Variable<bool>(domain, 2) );

    // Create row constraints
    for (int y = 0; y < rows; y++) {
        if (columns < 63) {
            if (columns < 31)
                CreateRowConstraint<63,512>(y);
            else
                CreateRowConstraint<63,1024>(y);
        } else {
            if (columns < 95)
                CreateRowConstraint<127,2048>(y);
            else
                CreateRowConstraint<127,4096>(y);
        }
    }

    // Create column constraints
    for (int x = 0; x < columns; x++) {
        if (rows < 63)
            if (rows < 31)
                CreateColumnConstraint<63,512>(x);
            else
                CreateColumnConstraint<63,1024>(x);
        else
            if (rows < 95)
                CreateColumnConstraint<127,2048>(x);
            else
                CreateColumnConstraint<127,4096>(x);
    }

    // Initialize hash random numbers
    srand(1);
    for (int v = 0; v < 4; v++)
        for (int i = 0; i < 128; i++) 
            hash_rand[v][i] = (long(rand()) << 32) + rand();
}

void Nonogram::ShowState(Variable<bool> *current)
{
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            if (grid[x][y]->GetDomainSize() == 1) {
                bool value = grid[x][y]->GetValue(0);
                putchar(value ? 'O' : '.');
            } else {
                putchar(' ');
            }
            putchar(current == grid[x][y] ? '*' : ' ');
        }
        printf("\n");
    }
}

void Nonogram::ShowSolution()
{
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            bool value = grid[x][y]->GetValue(0);
            printf("%c ", (value ? 'O' : '.'));
        }
        printf("\n");
    }
}

template <size_t M, size_t B>
void Nonogram::CreateRowConstraint(int y)
{
    RunLength<M,B> *r = new RunLength<M,B>(row_runs[y]);
    for (int x = 0; x < columns; x++) 
        r->AddVariable(grid[x][y]);
    AddConstraint(r);
}

template <size_t M, size_t B>
void Nonogram::CreateColumnConstraint(int x)
{
    RunLength<M,B> *r = new RunLength<M,B>(column_runs[x]);
    for (int y = 0; y < rows; y++)
        r->AddVariable(grid[x][y]);
    AddConstraint(r);
}

int main(int argc, char *argv[])
{
    bool rotate = false;

    Option option;
    option.sort = Option::SORT_FAILURES;
    option.GetOptions(argc, argv);
    if (optind < argc)
        rotate = atoi(argv[optind]) != 0;

    Nonogram puzzle(option, rotate);
    puzzle.Solve();

    return 0;
}

