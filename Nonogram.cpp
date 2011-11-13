#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#include "RunLength.h"
#include "Problem.h"

class Nonogram : public Problem<bool>
{
    public:
        Nonogram(Option option);
        void ShowState(Variable<bool> *);
        void ShowSolution();

    private:
        int  columns, rows;
        vector< vector<Variable<bool> * > >  grid;
        vector< vector<int> >  column_runs;
        vector< vector<int> >  row_runs;
};

Nonogram::Nonogram(Option option)
    : Problem<bool>(option)
{
    // Read input
    char line[80];
    fgets(line, sizeof(line), stdin);
    sscanf(line, "%d %d", &columns, &rows);

    column_runs.resize(columns);
    row_runs.resize(rows);
    for (int i = 0; i < columns + rows; i++) {
        fgets(line, sizeof(line), stdin);
        int n[16];
        int count = sscanf(line, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", 
                n+0, n+1, n+2, n+3, n+4, n+5, n+6, n+7, 
                n+8, n+9, n+10, n+11, n+12, n+13, n+14, n+15);

        for (int j = 0; j < count; j++) {
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

    // Create column constraints
    for (int x = 0; x < columns; x++) {
        RunLength *r = new RunLength(column_runs[x]);
        for (int y = 0; y < rows; y++)
            r->AddVariable(grid[x][y]);
        AddConstraint(r);
    }

    // Create row constraints
    for (int y = 0; y < rows; y++) {
        RunLength  *r = new RunLength(row_runs[y]);
        for (int x = 0; x < columns; x++) 
            r->AddVariable(grid[x][y]);
        AddConstraint(r);
    }
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

int main(int argc, char *argv[])
{
    Option option;
    option.sort = Option::SORT_FAILURES;
    option.GetOptions(argc, argv);

    Nonogram puzzle(option);
    puzzle.Solve();

    return 0;
}

