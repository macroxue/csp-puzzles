#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Function.h"
#include "Problem.h"

/*
   L1   L2    L3   L4    L5   XXX
   L6   XXX   L7   XXX   L8   XXX
   L9   L10   L11  L12   L13  L14
   L15  XXX   XXX  XXX   L16  XXX

   dog, run, top
   five, four, lost, mess, unit
   baker, forum, green, super
   prolog, vanish, wonder, yellow
 */

class Crossword : public Problem<char>
{
    public:
        Crossword(Option option);
        void ShowSolution();

    private:
        Variable<char>  *v[6][4];

        struct Word {
            bool operator()(int count, const char values[], char);
        };
};

Crossword::Crossword(Option option)
    : Problem<char>(option)
{
    // Letters as variables
    Variable<char> *l1, *l2, *l3, *l4, *l5, *l6, *l7, *l8, *l9, *l10,
             *l11, *l12, *l13, *l14, *l15, *l16;
    memset(v, 0, sizeof(v));
    v[0][0] = l1 = new Variable<char>("bfgs");
    v[1][0] = l2 = new Variable<char>("aoru");
    v[2][0] = l3 = new Variable<char>("krep");
    v[3][0] = l4 = new Variable<char>("eu");
    v[4][0] = l5 = new Variable<char>("rmn");
    v[0][1] = l6 = new Variable<char>("ioen");
    v[2][1] = l7 = new Variable<char>("ou");
    v[4][1] = l8 = new Variable<char>("ioen");
    v[0][2] = l9 = new Variable<char>("pvwy");
    v[1][2] = l10 = new Variable<char>("raoe");
    v[2][2] = l11 = new Variable<char>("onl");
    v[3][2] = l12 = new Variable<char>("lid");
    v[4][2] = l13 = new Variable<char>("ose");
    v[5][2] = l14 = new Variable<char>("ghrw");
    v[0][3] = l15 = new Variable<char>("erts");
    v[4][3] = l16 = new Variable<char>("erts");

    // Words as constraints
    Function<char, Word> *word1 = new Function<char, Word>;
    word1->AddVariable(5,l1,l2,l3,l4,l5);
    AddConstraint(word1);

    Function<char, Word> *word2 = new Function<char, Word>;
    word2->AddVariable(6,l9,l10,l11,l12,l13,l14);
    AddConstraint(word2);

    Function<char, Word> *word3 = new Function<char, Word>;
    word3->AddVariable(4,l1,l6,l9,l15);
    AddConstraint(word3);

    Function<char, Word> *word4 = new Function<char, Word>;
    word4->AddVariable(3,l3,l7,l11);
    AddConstraint(word4);

    Function<char, Word> *word5 = new Function<char, Word>;
    word5->AddVariable(4,l5,l8,l13,l16);
    AddConstraint(word5);
}

bool Crossword::Word::operator()(int count, const char values[], char)
{
    static const char *words[4][5] = {
        { "dog", "run", "top", NULL, NULL },
        { "five", "four", "lost", "mess", "unit" },
        { "baker", "forum", "green", "super", NULL },
        { "prolog", "vanish", "wonder", "yellow", NULL }
    };
    for (int i = 0; i < 5 && words[count-3][i]; i++)
        if (strncmp(values, words[count-3][i], count) == 0)
            return true;
    return false;
}

void Crossword::ShowSolution()
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 6; col++) {
            if (v[col][row]) 
                putchar(v[col][row]->GetValue(0));
            else
                putchar('.');
        }
        putchar('\n');
    }
}

int main(int argc, char *argv[])
{
    Option option;
    option.GetOptions(argc, argv);

    Crossword puzzle(option);
    puzzle.Solve();

    return 0;
}
