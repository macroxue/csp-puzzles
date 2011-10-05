#include <stdio.h>
#include <stdlib.h>

#include "OneToOne.h"
#include "Function.h"
#include "Problem.h"

const int MAX_SIZE = 64;

class Queens : public Problem<int>
{
    public:
        Queens(int size);
        void ShowSolution();

    private:
        static bool Distance(int count, const int values[], int distance);

        int  size;
        Variable<int>  *v[MAX_SIZE];
};

Queens::Queens(int size)
    : size(size)
{
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
            Function<int> *f = new Function<int>(Distance, y2-y1);
            f->AddVariable(2, v[y1], v[y2]);
            AddConstraint(f);
        }
}

bool Queens::Distance(int count, const int values[], int distance)
{
    return abs(values[0] - values[1]) != distance;
}

void Queens::ShowSolution()
{
    for (int row = 0; row < size; row++) {
        int value = v[row]->GetValue(0);
        for (int col = 0; col < size; col++)
            printf("%c ", (value == col ? 'Q' : '.'));
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int size = 4;
    if (argc > 1)
        size = atoi(argv[1]);
    if (size > MAX_SIZE) {
        printf("Max size is %d\n", MAX_SIZE);
        return 1;
    }

    Queens queens(size);
    queens.Solve(false);

    return 0;
}
