#include <stdio.h>
#include <stdlib.h>

#include "Different.h"
#include "Function.h"
#include "Problem.h"

class SendMoreMoney : public Problem<int>
{
    public:
        SendMoreMoney();
        void ShowSolution();

    private:
        static bool Condition1(int count, const int values[], int);
        static bool Condition(int count, const int values[], int);

        Variable<int> *s;
        Variable<int> *e;
        Variable<int> *n;
        Variable<int> *d;
        Variable<int> *m;
        Variable<int> *o;
        Variable<int> *r;
        Variable<int> *y;
        Variable<int> *v[8];
};

SendMoreMoney::SendMoreMoney()
{
    s = v[0] = new Variable<int>(1,9);
    e = v[1] = new Variable<int>(0,9);
    n = v[2] = new Variable<int>(0,9);
    d = v[3] = new Variable<int>(0,9);
    m = v[4] = new Variable<int>(1,9);
    o = v[5] = new Variable<int>(0,9);
    r = v[6] = new Variable<int>(0,9);
    y = v[7] = new Variable<int>(0,9);

    Different<int> *different = new Different<int>();
    for (unsigned i = 0; i < 8; i++)
        different->AddVariable(v[i]);
    AddConstraint(different);

    Function<int> *function1 = new Function<int>(Condition1, 0);
    function1->AddVariable(s);
    function1->AddVariable(m);
    function1->AddVariable(o);
    AddConstraint(function1);

    Function<int> *function = new Function<int>(Condition, 0);
    for (unsigned i = 0; i < 8; i++)
        function->AddVariable(v[i]);
    AddConstraint(function);
}

bool SendMoreMoney::Condition1(int count, const int values[], int)
{
    int sum1 = values[0] + values[1];
    int sum2 = values[1]*10 + values[2];
    return sum1 == sum2 || sum1 + 1 == sum2;
}

bool SendMoreMoney::Condition(int count, const int values[], int)
{
    return values[0]*1000 + values[1]*100 + values[2]*10 + values[3] 
        +  values[4]*1000 + values[5]*100 + values[6]*10 + values[1]
        == values[4]*10000 + values[5]*1000 + values[2]*100 + values[1]*10 + values[7];
}

void SendMoreMoney::ShowSolution()
{
    printf("     S=%d E=%d N=%d D=%d\n", 
            s->GetValue(0), e->GetValue(0), n->GetValue(0), d->GetValue(0));
    printf(" +)  M=%d O=%d R=%d E=%d\n", 
            m->GetValue(0), o->GetValue(0), r->GetValue(0), e->GetValue(0));
    printf("------------------------\n"); 
    printf(" M=%d O=%d N=%d E=%d Y=%d\n", 
            m->GetValue(0), o->GetValue(0), n->GetValue(0), e->GetValue(0), y->GetValue(0));
}

int main()
{
    SendMoreMoney  puzzle;
    puzzle.Solve(false);

    return 0;
}
