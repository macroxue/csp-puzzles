#ifndef RUNLENGTH_H
#define RUNLENGTH_H

#include "Constraint.h"
#include "Automaton.h"

//
// RunLength: 
//
template <size_t M>
class RunLength : public Constraint<bool>
{
    public:
        RunLength(vector<int> & length);
        bool OnDecided(Variable<bool> *decided);
        bool Enforce();

    private:
        vector<int> & length;
        Automaton<bool,2,M>  automaton;

};

template <size_t M>
RunLength<M>::RunLength(vector<int> & length)
    : length(length)
{
    typedef typename Automaton<bool,2,M>::Run   Run;
    vector<Run> run;

    for (size_t i = 0; i < length.size(); i++) {
        run.push_back(Run(false, i != 0, Run::AT_LEAST));
        run.push_back(Run(true, length[i], Run::EQUAL));
    }
    run.push_back(Run(false, 0, Run::AT_LEAST));

    new (&automaton) Automaton<bool,2,M>(run);
}

template <size_t M>
bool RunLength<M>::OnDecided(Variable<bool> *decided)
{
    return Enforce();
}

template <size_t M>
bool RunLength<M>::Enforce()
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_variables = variables.size();

    typedef typename Automaton<bool,2,M>::Input Input;
    Input input[num_variables];

    for (size_t i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            input[i].value = variables[i]->GetValue(0);
            input[i].decided = true;
        } else {
            input[i].decided = false;
        }
    }

    bool accepted = automaton.Accept(input, num_variables);
    if (!accepted)
        return false;

    for (size_t i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() > 1 && input[i].decided) {
            variables[i]->Decide(input[i].value);

            vector<Constraint<bool>*> &constraints = variables[i]->GetConstraints();
            for (size_t j = 0; j < constraints.size(); j++) {
                if (constraints[j] != this)
                    problem->ActivateConstraint(constraints[j]);
            }
        }
    }

    return true;
}

#endif

