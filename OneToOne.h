#ifndef ONETOONE_H
#define ONETOONE_H

#include "Different.h"

#include <map>
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
    int num_variables = variables.size();

    // Some value is in one and only one variable's domain.
    // TODO: more efficient tracking of undecided values
    typedef map<T, Variable<T> *>  value_map_t;
    value_map_t value_map;
    for (int i = 0; i < num_variables; i++) {
        Variable<T> *variable = variables[i];

        int domain_size = variable->GetDomainSize();
        for (int v = 0; v < domain_size; v++) {
            T value = variable->GetValue(v);
            typename value_map_t::iterator it = value_map.find(value);
            if (it == value_map.end()) {
                value_map.insert(make_pair(value, variable));
            } else {
                it->second = NULL;
            }
        }
    }

    typename value_map_t::iterator it;
    for (it = value_map.begin(); it != value_map.end(); it++) {
        T             value    = it->first;
        Variable<T> * variable = it->second;
        if (variable == NULL || variable->GetDomainSize() == 1)
            continue;

        variable->Decide(value);
        bool consistent = variable->PropagateDecision(NULL);
        if (!consistent)
            return false;
    }
    return true;
}

#endif
