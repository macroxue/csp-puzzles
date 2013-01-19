#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#include "RunLength.h"
#include "Problem.h"

__uint128_t hash_rand[2][2];

class Nonogram : public Problem<bool>
{
    public:
        Nonogram(Option option, bool rootate);
        void ShowState(Variable<bool> *);
        void ShowSolution();
        void ShowCounters();

    private:
        template <size_t M, size_t B>
            void CreateRowConstraint(int y);
        template <size_t M, size_t B>
            void CreateColumnConstraint(int x);

        int  columns, rows;
        vector< vector<Variable<bool> > >  grid;
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
    for (int i = 0; i < rows; i++)
        row_runs[i].reserve((columns+1)/2);
    column_runs.resize(columns);
    for (int i = 0; i < columns; i++)
        column_runs[i].reserve((rows+1)/2);
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
    grid.resize(columns);
    for (int x = 0; x < columns; x++) {
        grid[x].reserve(rows);
        for (int y = 0; y < rows; y++)
            new (&grid[x][y]) Variable<bool>(false, true);
    }

    // Create row constraints
    for (int y = 0; y < rows; y++) {
        if (columns < 31)
            CreateRowConstraint<31,9>(y);
        else if (columns < 63) 
            CreateRowConstraint<63,10>(y);
        else if (columns < 95)
            CreateRowConstraint<95,11>(y);
        else
            CreateRowConstraint<127,12>(y);
    }

    // Create column constraints
    for (int x = 0; x < columns; x++) {
        if (rows < 31)
            CreateColumnConstraint<31,9>(x);
        else if (rows < 63)
            CreateColumnConstraint<63,10>(x);
        else if (rows < 95)
            CreateColumnConstraint<95,11>(x);
        else
            CreateColumnConstraint<127,12>(x);
    }

    // Initialize hash random numbers
    srand(1);
    for (int v = 0; v < 2; v++)
        for (int i = 0; i < 2; i++) {
            hash_rand[v][i] = ((__uint128_t)rand() << 96)
                + ((__uint128_t)rand() << 64) + ((__uint128_t)rand() << 32) + rand();
            // make sure it's an odd number
            hash_rand[v][i] = (hash_rand[v][i] << 1) + 1;
        }

    // Initialize counters
    counters[0].name = "Cache lookups";
    counters[1].name = "Cache hits";
}

void Nonogram::ShowState(Variable<bool> *current)
{
    char line[columns * 2 + 1];
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            if (grid[x][y].GetDomainSize() == 1) {
                bool value = grid[x][y].GetValue(0);
                line[x * 2] = (value ? 'O' : '.');
            } else {
                line[x * 2] = ' ';
            }
            line[x * 2 + 1] = (current == &grid[x][y] ? '*' : ' ');
        }
        line[columns * 2] = '\0';
        puts(line);
    }
}

void Nonogram::ShowSolution()
{
    char line[columns * 2 + 1];
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            bool value = grid[x][y].GetValue(0);
            line[x * 2] = (value ? 'O' : '.');
            line[x * 2 + 1] = ' ';
        }
        line[columns * 2] = '\0';
        puts(line);
    }
}

void Nonogram::ShowCounters()
{
    Problem<bool>::ShowCounters();
    printf("Hit ratio: %.1f%%\n", 100.0*counters[1].value/counters[0].value);
}

template <size_t M, size_t B>
void Nonogram::CreateRowConstraint(int y)
{
    RunLength<M,B> *r = new RunLength<M,B>(row_runs[y]);
    for (int x = 0; x < columns; x++) 
        r->AddVariable(&grid[x][y]);
    AddConstraint(r);
}

template <size_t M, size_t B>
void Nonogram::CreateColumnConstraint(int x)
{
    RunLength<M,B> *r = new RunLength<M,B>(column_runs[x]);
    for (int y = 0; y < rows; y++)
        r->AddVariable(&grid[x][y]);
    AddConstraint(r);
}

int main(int argc, char *argv[])
{
    bool rotate = false;

    Option option;
    option.sort = Option::SORT_FAILURES;
    option.arc_consistency = true;
    option.GetOptions(argc, argv);
    if (optind < argc)
        rotate = atoi(argv[optind]) != 0;

    Nonogram puzzle(option, rotate);
    puzzle.Solve();

    return 0;
}

