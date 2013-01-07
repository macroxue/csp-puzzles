#include <stdio.h>
#include "Automaton.h"

const size_t M = 12;
typedef Automaton<bool,2,M>::Run   Run;

class Line : public Automaton<bool,2,M>::Input<Line> {
    public:
        bool operator ==(const Line &line) const;
        void Show() const;

        bool IsDecided(size_t i) const  { return decided.Has(i); }
        void SetDecided(size_t i)       { decided.Add(i); }
        bool GetValue(size_t i) const   { return value.Has(i); }
        void SetValue(size_t i, bool v) { if (v) value.Add(i); else value.Remove(i); }

        uint64_t Hash() const;

    private:
        Set<M+1> value;
        Set<M+1> decided;
};


void test(Automaton<bool,2,M> &a, Line &input, size_t input_size)
{
    bool accepted = a.Accept(input, input_size);

    printf("%s: ", (accepted ? "Accepted" : "Rejected"));
    for (size_t i = 0; i < input_size; i++)
        if (input.IsDecided(i)) 
            printf("%d ", input.GetValue(i));
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

    Automaton<bool,2,M> a(run);

    for (size_t len = M/2; len <= M; len++) {
        Line input;
        test(a, input, len);
    }

    size_t len = M;
    for (size_t pos = 0; pos < len; pos++) {
        Line input;
        input.SetValue(pos, true);
        input.SetDecided(pos);
        test(a, input, len);
    }

    len = M-1;
    for (size_t pos = 0; pos < len; pos++) {
        Line input;
        input.SetValue(pos, false);
        input.SetDecided(pos);
        test(a, input, len);
    }

    return 0;
}

