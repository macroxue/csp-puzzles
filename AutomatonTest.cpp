#include <stdio.h>
#include "Automaton.h"

typedef Automaton<bool,2>::Run   Run;
typedef Automaton<bool,2>::Input Input;

void test(Automaton<bool,2> &a, Input input[], size_t input_size)
{
    bool accepted = a.Accept(input, input_size);

    printf("%s: ", (accepted ? "Accepted" : "Rejected"));
    for (size_t i = 0; i < input_size; i++)
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

    Automaton<bool,2> a(run);
    const size_t MAX_LEN = 12;
    Input input[MAX_LEN];

    for (size_t len = MAX_LEN/2; len <= MAX_LEN; len++) {
        for (size_t i = 0; i < len; i++)
            input[i] = Input(false, false);
        test(a, input, len);
    }

    size_t len = MAX_LEN;
    for (size_t pos = 0; pos < len; pos++) {
        for (size_t i = 0; i < len; i++)
            input[i] = Input(false, false);
        input[pos].value   = true;
        input[pos].decided = true;
        test(a, input, len);
    }

    len = MAX_LEN-1;
    for (size_t pos = 0; pos < len; pos++) {
        for (size_t i = 0; i < len; i++)
            input[i] = Input(false, false);
        input[pos].value   = false;
        input[pos].decided = true;
        test(a, input, len);
    }

    return 0;
}

