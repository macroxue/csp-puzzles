#include <stdio.h>
#include <stdlib.h>

#include "FunctionAC.h"
#include "Problem.h"

const int MAX_SIZE = 64;

class Fiver : public Problem<bool>
{
    public:
        Fiver(int size);
        void ShowSolution();

    private:
        int  size;
        Variable<bool>  *v[MAX_SIZE][MAX_SIZE];

        struct Xor
        {
            bool operator() (bool x, bool y) const { return x ^ y; }
        };
};

Fiver::Fiver(int size)
    : size(size)
{
    // Variables
    bool domain[2] = { false, true };
    for (int y = 0; y < size; y++)
        for (int x = 0; x < size; x++)
            v[x][y] = new Variable<bool>(domain, 2);

    // Constraints with neighbors
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            FunctionAC<bool, Xor, Xor> *c = new FunctionAC<bool, Xor, Xor>(true);
            c->AddVariable(v[x][y]);
            if (x > 0)
                c->AddVariable(v[x-1][y]);
            if (y > 0)
                c->AddVariable(v[x][y-1]);
            if (x < size - 1)
                c->AddVariable(v[x+1][y]);
            if (y < size - 1)
                c->AddVariable(v[x][y+1]);
            AddConstraint(c);
        }
    }
}

void Fiver::ShowSolution()
{
    int moves = 0;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            printf("%d ", v[x][y]->GetValue(0));
            moves += v[x][y]->GetValue(0);
        }
        printf("\n");
    }
    printf("%d moves\n", moves);
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

    Fiver fiver(size);
    fiver.Solve(false);

    return 0;
}
