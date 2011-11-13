#ifndef RUNLENGTH_H
#define RUNLENGTH_H

#include "Constraint.h"
#include "Automaton.h"

//
// RunLength: 
//
class RunLength : public Constraint<bool>
{
    public:
        RunLength(vector<int> & length);
        bool OnDecided(Variable<bool> *decided);
        bool Enforce();

    private:
        vector<int> & length;
        Automaton<bool>  automaton;
};

RunLength::RunLength(vector<int> & length)
    : length(length)
{
    typedef Automaton<bool>::Run   Run;
    vector<Run> run;

    for (size_t i = 0; i < length.size(); i++) {
        run.push_back(Run(false, i != 0, Run::AT_LEAST));
        run.push_back(Run(true, length[i], Run::EQUAL));
    }
    run.push_back(Run(false, 0, Run::AT_LEAST));

    new (&automaton) Automaton<bool>(run);
}

bool RunLength::OnDecided(Variable<bool> *decided)
{
    return Enforce();
}

bool RunLength::Enforce()
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_variables = variables.size();

    typedef Automaton<bool>::Input Input;
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

    size_t num_decided = 0;
    size_t decided_index[num_variables];
    for (size_t i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() > 1 && input[i].decided) {
            variables[i]->Decide(input[i].value);

            decided_index[num_decided++] = i;
        }
    }

    for (size_t i = 0; i < num_decided; i++) {
        vector<Constraint<bool>*> &constraints = 
            variables[ decided_index[i] ]->GetConstraints();
        for (size_t j = 0; j < constraints.size(); j++) {
            if (constraints[j] == this)
                continue;
#if 1
            problem->ActivateConstraint(constraints[j]);
#else
            if (!constraints[j]->OnDecided(variables[i]))
                return false;
#endif
        }
    }

    return true;
}

#endif

