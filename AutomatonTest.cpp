#include <stdio.h>
#include "Automaton.h"

typedef Automaton<bool>::Run   Run;
typedef Automaton<bool>::Input Input;

void test(Automaton<bool> &a, Input input[], int input_size)
{
    bool accepted = a.Accept(input, input_size);

    printf("%s: ", (accepted ? "Accepted" : "Rejected"));
    for (int i = 0; i < input_size; i++)
        if (input[i].decided) 
            printf("%d ", input[i].value);
        else
            printf("? ");
    printf("\n");
}

int main()
{
    vector<Run> run;
    run.push_back(Run(false, 0, Run::AT_LEAST));
    run.push_back(Run(true,  3, Run::EQUAL));
    run.push_back(Run(false, 1, Run::AT_LEAST));
    run.push_back(Run(true,  4, Run::EQUAL));
    run.push_back(Run(false, 0, Run::AT_LEAST));

    Automaton<bool> a(run);
    const int MAX_LEN = 12;
    Input input[MAX_LEN];

    for (int len = MAX_LEN/2; len <= MAX_LEN; len++) {
        for (int i = 0; i < len; i++)
            input[i] = Input(false, false);
        test(a, input, len);
    }

    int len = MAX_LEN;
    for (int pos = 0; pos < len; pos++) {
        for (int i = 0; i < len; i++)
            input[i] = Input(false, false);
        input[pos].value   = true;
        input[pos].decided = true;
        test(a, input, len);
    }

    len = MAX_LEN-1;
    for (int pos = 0; pos < len; pos++) {
        for (int i = 0; i < len; i++)
            input[i] = Input(false, false);
        input[pos].value   = false;
        input[pos].decided = true;
        test(a, input, len);
    }

    return 0;
}

