#ifndef NEXT_H
#define NEXT_H

#include "Constraint.h"

//
// Next: Two variables must take values next to each other.
//
template <class T>
class Next : public Constraint<T>
{
    public:
        bool OnDecided(Variable<T> *decided);

    private:
};

template <class T>
bool Next<T>::OnDecided(Variable<T> *decided)
{
    assert(decided->GetDomainSize() == 1);

    // Remove used value from other variables' domains
    vector<Variable<T> *>  &variables = Constraint<T>::variables;
    assert(variables.size() == 2);
    int num_variables = variables.size();
    int value = decided->GetValue(0);

    for (int i = 0; i < num_variables; i++) {
        if (variables[i] == decided) continue;

        int old_domain_size = variables[i]->GetDomainSize();
        for (int j = 0; j < old_domain_size; j++) {
            int value2 = variables[i]->GetValue(j);
            if (value2 - value != 1 && value2 - value != -1)
                variables[i]->Exclude(value2);
        }
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
