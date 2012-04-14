#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "Different.h"
#include "Function.h"
#include "Problem.h"

const int MAX_PEGS    = 9;
const int MAX_GUESSES = 32;

int  num_pegs   = 4;
int  num_colors = 7;
bool auto_play  = false;

struct Code {
    int num_correct;
    int num_incorrect;
    int pegs[MAX_PEGS];

    void Randomize()
    {
        int colors[num_colors];
        for (int i = 0; i < num_colors; i++)
            colors[i] = i;
        for (int i = 0; i < num_colors-1; i++) {
            int pos = rand() % (num_colors-i);
            std::swap(colors[num_colors-1-i], colors[pos]);
        }
        for (int i = 0; i < num_pegs; i++)
            pegs[i] = colors[i];

    }

    void Match(const int guess_pegs[], int &num_correct, int &num_incorrect)
    {
        num_correct = num_incorrect = 0;
        for (int i = 0; i < num_pegs; i++) {
            if (pegs[i] == guess_pegs[i])
                num_correct++;
            else {
                for (int j = 0; j < num_pegs; j++) 
                    if (pegs[j] == guess_pegs[i]) {
                        num_incorrect++;
                        break;
                    }
            }
        }
    }
};

Code guesses[MAX_GUESSES];
int  num_guesses = 0;

class MasterMind : public Problem<int>
{
    public:
        MasterMind(Option option, int num_pegs, int num_colors,
                int num_guesses, Code guesses[]);
        void ShowSolution();

    private:
        int  num_pegs;
        int  num_colors;
        Variable<int>  **v;

        struct Match
        {
            bool operator()(int count, const int values[], int guess_id);
        };
};

MasterMind::MasterMind(Option option, int num_pegs, int num_colors,
        int num_guesses, Code guesses[])
    : Problem<int>(option), num_pegs(num_pegs), num_colors(num_colors)
{
    v = new Variable<int> * [num_pegs];

    // Variables
    for (int i = 0; i < num_pegs; i++)
        v[i] = new Variable<int>(0, num_colors-1);

    // Each peg must have a unique color
    Different<int> *c = new Different<int>;
    for (int i = 0; i < num_pegs; i++)
        c->AddVariable(v[i]);
    AddConstraint(c);

    // Add guesses as constraints
    for (int i = 0; i < num_guesses; i++) {
        Function<int, Match> *match = new Function<int, Match>(i);
        for (int i = 0; i < num_pegs; i++)
            match->AddVariable(v[i]);
        AddConstraint(match);
    }
}

void MasterMind::ShowSolution()
{
    Code &guess = guesses[num_guesses];
    for (int i = 0; i < num_pegs; i++) {
        int value = v[i]->GetValue(0);
        printf("%d ", value+1);
        guess.pegs[i] = value;
    }
    num_guesses++;
}

bool MasterMind::Match::operator()(int count, const int values[], int guess_id)
{
    Code &guess = guesses[guess_id];
    int num_correct, num_incorrect;
    guess.Match(values, num_correct, num_incorrect);
    return num_correct == guess.num_correct && num_incorrect == guess.num_incorrect;
}

int main(int argc, char *argv[])
{
    Option option;
    option.GetOptions(argc, argv);
    option.interactive   = true;
    option.num_solutions = 1;
    if (optind < argc)
        num_pegs = atoi(argv[optind]);
    if (optind + 1 < argc)
        num_colors = atoi(argv[optind+1]);
    if (optind + 2 < argc) {
        auto_play = true;
        srand( atoi(argv[optind+2]) );
    }

    int total_guesses = 0, max_guesses = 0;
    for (int game = 1; game <= 1000; game++) {
        Code target;
        target.Randomize();
        num_guesses = 0;
        for (int round = 1; ; round++) {
            // Guess
            printf("Guess #%d: ", round);
            MasterMind puzzle(option, num_pegs, num_colors, num_guesses, guesses);
            puzzle.Solve();
            if (num_guesses < round) {
                printf("No solution.\n");
                break;
            }

            // Get match counts
            Code &guess = guesses[num_guesses-1];
            if (auto_play) {
                target.Match(guess.pegs, guess.num_correct, guess.num_incorrect);
                printf("--> %d%d\n", guess.num_correct, guess.num_incorrect);
            } else {
                printf("--> ");
                int num_items;
                do num_items = scanf("%1d%1d", &guess.num_correct, &guess.num_incorrect);
                while (num_items != 2);
            }
            if (guess.num_correct == num_pegs) {
                printf("Great! I solved the puzzle in %d guesses.\n", round);
                total_guesses += round;
                if (max_guesses < round)
                    max_guesses = round;
                break;
            }
        }
        printf("%d games played, average guesses = %.2f, maximum guesses = %d\n",
                game, (double)total_guesses/game, max_guesses);
    }

    return 0;
}

