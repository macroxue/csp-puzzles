#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "OneToOne.h"
#include "Function.h"
#include "Problem.h"

class Queens : public Problem<int>
{
    public:
        Queens(int size);
        void ShowSolution();

    private:
        int  size;
        Variable<int>  **v;

        struct Safe
        {
            bool operator()(int count, const int values[], int distance);
        };
};

Queens::Queens(int size)
    : size(size)
{
    v = new Variable<int> * [size];

    // Variables
    for (int row = 0; row < size; row++) {
        v[row] = new Variable<int>(0, size-1);
        v[row]->SetId(row);
    }

    // Each queen must be on a unique row
    OneToOne<int> *c = new OneToOne<int>;
    for (int row = 0; row < size; row++)
        c->AddVariable(v[row]);
    AddConstraint(c);

    // No two queens can attack each other diagonally
    for (int y1 = 0; y1 < size-1; y1++)
        for (int y2 = y1+1; y2 < size; y2++) {
            Function<int, Safe> *safe = new Function<int, Safe>(y2-y1);
            safe->AddVariable(2, v[y1], v[y2]);
            AddConstraint(safe);
        }
}

bool Queens::Safe::operator()(int count, const int values[], int distance)
{
    return abs(values[0] - values[1]) != distance;
}

int solutions_required = INT_MAX;

void Queens::ShowSolution()
{
    for (int row = 0; row < size; row++) {
        int value = v[row]->GetValue(0);
        for (int col = 0; col < size; col++)
            printf("%c ", (value == col ? 'Q' : '.'));
        printf("\n");
    }
    if (num_solutions >= solutions_required)
        exit(0);
}

int main(int argc, char *argv[])
{
    int size = 4;
    if (argc > 1)
        size = atoi(argv[1]);
    if (argc > 2)
        solutions_required = atoi(argv[2]);

    Queens queens(size);
    queens.Solve(true);

    return 0;
}
