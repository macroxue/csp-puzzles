#ifndef FUNCTION_H
#define FUNCTION_H

#include "Constraint.h"

//
// Function: F(x1, x2, ..., xn, y) -> bool
//
template <class T, class F>
class Function : public Constraint<T>
{
    public:
        Function(T target);
        bool OnDecided(Variable<T> *decided);

    private:
        T    target;
};

template <class T, class F>
Function<T,F>::Function(T target)
    : target(target)
{
}

template <class T, class F>
bool Function<T,F>::OnDecided(Variable<T> *decided)
{
    vector<Variable<T> *>  &variables = Constraint<T>::variables;
    int num_variables = variables.size();
    int num_decided = 0;
    int undecided_index = 0;

    T values[num_variables];
    for (int i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            values[i] = variables[i]->GetValue(0);
            num_decided++;
        } else {
            undecided_index = i;
        }
    }

    if (num_decided < num_variables - 1)
        return true;

    F   condition_fn;
    if (num_decided == num_variables) {
        // TODO: Why is this needed? Should be true always.
        // Constraint loop caused by depth first search!
        return condition_fn(num_variables, values, target);
    }

    // Only one variable is undecided. Apply the constraint.
    Variable<T> *undecided = variables[undecided_index];

    int old_domain_size = undecided->GetDomainSize();
    for (int i = 0; i < undecided->GetDomainSize(); i++) {
        T value = undecided->GetValue(i);
        values[undecided_index] = value;
        bool consistent = condition_fn(num_variables, values, target);
        if (!consistent)
            undecided->Exclude(value);
    }

    int new_domain_size = undecided->GetDomainSize();
    if (new_domain_size == 0)
        return false;

    if (old_domain_size > 1 && new_domain_size == 1) {
        // One constraint can decide multiple variables.
        // TODO: If there are two variables, this constraint can't be reused.
        //bool consistent = undecided->PropagateDecision((num_variables == 2) ? this : NULL);
        bool consistent = undecided->PropagateDecision(NULL);
        if (!consistent)
            return false;
    }

    return true;
}

#endif
