#ifndef ONETOONE_H
#define ONETOONE_H

#include "Different.h"

using namespace std;;

//
// OneToOne: Each value is uniquely assigned to one variable.
//
template <class T>
class OneToOne : public Different<T>
{
    public:
        bool OnReduced(Variable<T> *reduced);

    private:
};

template <class T>
bool OneToOne<T>::OnReduced(Variable<T> *reduced)
{
    vector<Variable<T> *>  &variables = Constraint<T>::variables;
    size_t num_variables = variables.size();

    // Some value is in one and only one variable's domain.
    // TODO: more efficient tracking of undecided values
    T  low, high;
    this->GetBounds(low, high);
    Variable<T> * value_map[high - low + 1];
    for (T  value = low; value <= high; value++)
        value_map[value - low] = NULL;

    Variable<T> *MULTIPLE = (Variable<T> *)0x1;
    for (size_t i = 0; i < num_variables; i++) {
        Variable<T> *variable = variables[i];

        size_t domain_size = variable->GetDomainSize();
        for (size_t v = 0; v < domain_size; v++) {
            T value = variable->GetValue(v);
            if (value_map[value - low] == NULL) {
                value_map[value - low] =  variable;
            } else {
                value_map[value - low] = MULTIPLE;
            }
        }
    }

    for (T value = low; value <= high; value++) {
        Variable<T> * variable = value_map[value - low];
        if (variable == NULL || variable == MULTIPLE
                || variable->GetDomainSize() == 1)
            continue;

        variable->Decide(value);
        if (variable->GetDomainSize() == 0)
            return false;

        bool consistent = variable->PropagateDecision(NULL);
        if (!consistent)
            return false;
    }
    return true;
}

#endif
