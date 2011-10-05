#ifndef CONSECUTIVE_H
#define CONSECUTIVE_H

#include "Constraint.h"

//
// Consecutive: All variables must have consecutive values increasingly.
//
template <class T>
class Consecutive : public Constraint<T>
{
    public:
        bool OnDecided(Variable<T> *decided);

    private:
};

template <class T>
bool Consecutive<T>::OnDecided(Variable<T> *decided)
{
    assert(decided->GetDomainSize() == 1);

    // Remove used value from other variables' domains
    vector<Variable<T> *>  &variables = Constraint<T>::variables;
    int num_variables = variables.size();
    int value = decided->GetValue(0);

    int offset = 0;
    for (int i = 0; i < num_variables; i++)
        if (variables[i] == decided) {
            offset = i;
            break;
        }

    for (int i = 0; i < num_variables; i++) {
        if (variables[i] == decided) continue;

        int old_domain_size = variables[i]->GetDomainSize();
        variables[i]->Decide(value - offset + i);
        int new_domain_size = variables[i]->GetDomainSize();

        if (new_domain_size == 0)
            return false;
        if (old_domain_size > 1 && new_domain_size == 1) {
            // One constraint can decide multiple variables,
            // so include it in the propagation.
            bool consistent = variables[i]->PropagateDecision(NULL);
            if (!consistent)
                return false;
        } 
    }
    return true;
}

#endif
