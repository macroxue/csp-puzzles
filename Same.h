#ifndef SAME_H
#define SAME_H

#include "Constraint.h"

//
// Same: All variables must take the same value.
//
template <class T>
class Same : public Constraint<T>
{
    public:
        bool OnDecided(Variable<T> *decided);

    private:
};

template <class T>
bool Same<T>::OnDecided(Variable<T> *decided)
{
    assert(decided->GetDomainSize() == 1);

    // Apply the same value to other variables' domains
    vector<Variable<T> *>  &variables = Constraint<T>::variables;
    int num_variables = variables.size();
    int value = decided->GetValue(0);

    for (int i = 0; i < num_variables; i++) {
        if (variables[i] == decided) continue;

        int old_domain_size = variables[i]->GetDomainSize();
        variables[i]->Decide(value);
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
