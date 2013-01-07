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

Code target;
Code guesses[MAX_GUESSES];
int  num_guesses = 0;

class MasterMind : public Problem<int>
{
    public:
        MasterMind(Option option, int num_pegs, int num_colors);
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

MasterMind::MasterMind(Option option, int num_pegs, int num_colors)
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
}

void MasterMind::ShowSolution()
{
    printf("Guess #%d: ", num_guesses+1);
    Code &guess = guesses[num_guesses];
    for (int i = 0; i < num_pegs; i++) {
        int value = v[i]->GetValue(0);
        printf("%d ", value+1);
        guess.pegs[i] = value;
    }
    num_guesses++;

    // Get match counts
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
        printf("Great! I solved the puzzle in %d guesses.\n", num_guesses);
        option.num_solutions = 1;
    } else {
        // Add guesses as constraints
        Function<int, Match> *match = new Function<int, Match>(num_guesses-1);
        for (int i = 0; i < num_pegs; i++)
            match->AddVariable(v[i]);
        AddConstraint(match);
    }
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
        target.Randomize();
        num_guesses = 0;
        MasterMind puzzle(option, num_pegs, num_colors);
        puzzle.Solve();

        if (guesses[num_guesses-1].num_correct < num_pegs) {
            printf("No solution.\n");
        } else {
            total_guesses += num_guesses;
            if (max_guesses < num_guesses)
                max_guesses = num_guesses;
            printf("%d games played, average guesses = %.2f, maximum guesses = %d\n",
                    game, (double)total_guesses/game, max_guesses);
        }
    }

    return 0;
}

