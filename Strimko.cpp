#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OneToOne.h"
#include "Problem.h"

const int M = 8;

class Strimko : public Problem<int>
{
    public:
        Strimko(Option option);
        void ShowSolution();

    private:
        int             size;
        Variable<int>  *v[M][M];
        char            layout[M*2][M*2];
};

Strimko::Strimko(Option option)
    : Problem<int>(option)
{
    memset(layout, ' ', sizeof(layout));

    // Read the layout
    size = M;
    for (int y = 0; y < size*2-1; y++) {
        for (int x = 0; x < size*2; x++) {
            char ch = getchar();
            if (ch == '\n') {
                if (size == M)
                    size = x/2 + 1;
                break;
            }
            layout[x][y] = ch;
        }
    }

    // Create variables
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            char ch = layout[x*2][y*2];
            if (ch == 'O') {
                v[x][y] = new Variable<int>(1, size);
            } else {
                int value = ch - '0';
                v[x][y] = new Variable<int>(value, value);
            }
        }
    }

    // Constraints on rows
    for (int y = 0; y < size; y++) {
        OneToOne<int> *c = new OneToOne<int>();
        for (int x = 0; x < size; x++)
            c->AddVariable(v[x][y]);
        AddConstraint(c);
    }

    // Constraints on columns
    for (int x = 0; x < size; x++) {
        OneToOne<int> *c = new OneToOne<int>();
        for (int y = 0; y < size; y++)
            c->AddVariable(v[x][y]);
        AddConstraint(c);
    }

    // Constraints on streams
    int num_streams = 0;
    int stream[M][M];
    int merge[M*M];
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            stream[x][y] = -1;
            if (x > 0 && layout[x*2-1][y*2] == '-') { 
                // Same stream as the left neighbor
                if (stream[x][y] == -1)
                    stream[x][y] = stream[x-1][y];
                else
                    merge[ stream[x][y] ] = stream[x-1][y];
            } 
            if (y > 0 && layout[x*2][y*2-1] == '|') {
                // Same stream as the top neighbor
                if (stream[x][y] == -1)
                    stream[x][y] = stream[x][y-1];
                else
                    merge[ stream[x][y] ] = stream[x][y-1];
            } 
            if (x > 0 && y > 0 && strchr("\\X", layout[x*2-1][y*2-1])) {
                // Same stream as the top-left neighbor
                if (stream[x][y] == -1)
                    stream[x][y] = stream[x-1][y-1];
                else
                    merge[ stream[x][y] ] = stream[x-1][y-1];
            } 
            if (x < size - 1 && y > 0 && strchr("/X", layout[x*2+1][y*2-1])) {
                // Same stream as the top-right neighbor
                if (stream[x][y] == -1)
                    stream[x][y] = stream[x+1][y-1];
                else
                    merge[ stream[x][y] ] = stream[x+1][y-1];
            } 

            // Create a new stream if it doesn't belong to any existing.
            if (stream[x][y] == -1) {
                stream[x][y] = num_streams;
                merge[num_streams] = -1;
                num_streams++;
            }
        }
    }

    OneToOne<int> *c[num_streams];
    memset(c, 0, sizeof(c));

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            // Find what the stream merges into
            int s = stream[x][y];
            while (merge[s] != -1)
                s = merge[s];

            if (c[s] == NULL)
                c[s] = new OneToOne<int>();
            c[s]->AddVariable(v[x][y]);
        }
    }

    for (int i = 0; i < num_streams; i++) {
        // Merged streams are avoided
        if (c[i] != NULL)
            AddConstraint(c[i]);
    }
}

void Strimko::ShowSolution()
{
    for (int y = 0; y < size*2-1; y++) {
        for (int x = 0; x < size*2-1; x++) {
            if (x % 2 == 0 && y % 2 == 0)
                putchar('0' + v[x/2][y/2]->GetValue(0));
            else
                putchar(layout[x][y]);
        }
        putchar('\n');
    }
}

int main(int argc, char *argv[])
{
    Option option;
    option.GetOptions(argc, argv);

    Strimko puzzle(option);
    puzzle.Solve();

    return 0;
}

