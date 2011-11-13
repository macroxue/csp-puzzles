#ifndef FUNCTIONAC_H
#define FUNCTIONAC_H

#include "Constraint.h"

//
// FunctionAC: O(x1, x2, ..., xn, y) -> bool
//
// T: Type of values
// O: Associative and commutative binary operation
// I: Inverse operation of O, i.e. O(a,b)=c <=> I(c,a)=b 
//
template <class T, class O, class I>
class FunctionAC : public Constraint<T>
{
    public:
        FunctionAC(T target);
        bool OnDecided(Variable<T> *decided);

    private:
        T    target;
};

template <class T, class O, class I>
FunctionAC<T, O, I>::FunctionAC(T target)
    : target(target)
{
}

template <class T, class O, class I>
bool FunctionAC<T, O, I>::OnDecided(Variable<T> *decided)
{
    vector<Variable<T> *>  &variables = Constraint<T>::variables;
    size_t num_variables = variables.size();
    size_t num_decided = 0;
    size_t undecided_index = 0;

    O   op;
    T   result = 0;
    for (size_t i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            T value = variables[i]->GetValue(0);
            result = op(result, value);
            num_decided++;
        } else {
            undecided_index = i;
        }
    }

    if (num_decided < num_variables - 1)
        return true;

    if (num_decided == num_variables) {
        // TODO: Why is this needed? Should be true always.
        return result == target;
    }

    // Only one variable is undecided. Apply the constraint.
    Variable<T> *undecided = variables[undecided_index];
    I  inverse_op;
    T  target_value = inverse_op(target, result);

    // Target value must be in the domain. No need to check.
    // TODO: true?
    undecided->Decide(target_value);

    // One constraint can only decide at most one variable,
    // so exclude it from propagation.
    return undecided->PropagateDecision(this);
}

#endif
