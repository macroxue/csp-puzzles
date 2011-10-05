#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "OneToOne.h"
#include "Problem.h"

class Sudoku : public Problem<int>
{
    public:
        Sudoku();
        void ShowState(Variable<int> *current);
        void ShowSolution();

    private:
        Variable<int>  *v[9][9];
};

Sudoku::Sudoku()
{
    // Variables 9x9
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            // Read input
            int value;
            do value = getchar();
            while (value != '.' && !isdigit(value));
            if (value == '.') value = 0;
            else value -= '0';

            // Variable domain based on input
            if (value == 0)
                v[x][y] = new Variable<int>(1, 9);
            else
                v[x][y] = new Variable<int>(value, value);
        }
    }

    // Constraints on rows
    for (int y = 0; y < 9; y++) {
        OneToOne<int> *c = new OneToOne<int>();
        for (int x = 0; x < 9; x++)
            c->AddVariable(v[x][y]);
        AddConstraint(c);
    }

    // Constraints on columns
    for (int x = 0; x < 9; x++) {
        OneToOne<int> *c = new OneToOne<int>();
        for (int y = 0; y < 9; y++)
            c->AddVariable(v[x][y]);
        AddConstraint(c);
    }

    // Constraints on 3x3 squares
    for (int q = 0; q < 3; q++) {
        for (int p = 0; p < 3; p++) {
            OneToOne<int> *c = new OneToOne<int>();
            for (int y = q * 3; y < q * 3 + 3; y++)  {
                for (int x = p * 3; x < p * 3 + 3; x++) 
                    c->AddVariable(v[x][y]);
            }
            AddConstraint(c);
        }
    }
}

void Sudoku::ShowState(Variable<int> *current)
{
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            int domain_size = v[x][y]->GetDomainSize();
            if (domain_size > 1) {
                if (v[x][y] == current)
                    putchar('[');
                int num_cand = 0;
                for (int i = 0; i < domain_size; i++) {
                    int value = v[x][y]->GetValue(i);
                    putchar(value + '0');
                    num_cand++;
                }
                for (;num_cand < 9; num_cand++)
                    putchar(' ');
                if (v[x][y] == current)
                    putchar(']');
            } else {
                printf("%c        ", v[x][y]->GetValue(0) + '0');
            }
            printf( x == 2 || x == 5 ? " | " : " ");
        }
        puts(y == 2||y == 5 ? 
            "\n------------------------------+-------------------------------+------------------------------" : "");
    }
    putchar('\n');
}

void Sudoku::ShowSolution()
{
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            int value = v[x][y]->GetValue(0);
            putchar(' ');
            putchar(value == 0 ? '.' : value + '0');
            printf( x == 2 || x == 5 ? " |" : "");
        }
        puts(y == 2 || y == 5 ? "\n ------+-------+------" : "");
    }
    putchar('\n');
    exit(0);
}

int main(int argc, char *argv[])
{
    Sudoku sudoku;
    sudoku.Solve(true);

    return 0;
}

