#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <vector>
using namespace std;

template <class T>
class Variable;

template <class T>
class Problem;

// 
// A constraint is about the relation of a set of variables
//
template <class T>
class Constraint
{
    public:
        void AddVariable(Variable<T> *variable);
        void AddVariable(int num_variables, ...);

        void UpdateBounds();
        void GetBounds(T &low, T &high) const;

        // Enforce the constraint.
        // The domains can be reduced to meet the constraint.
        // False is returned if the constraint cannot be met.
        virtual bool OnDecided(Variable<T> *decided);
        virtual bool OnReduced(Variable<T> *reduced) { return true; }

        Problem<T> *GetProblem() const { return problem; }

    protected:
        vector<Variable<T> *>  variables;

        T    low_value, high_value;

        friend class Problem<T>;
        Problem<T>  *problem;
};

#include "Variable.h"

#include <stdarg.h>

template <class T>
void Constraint<T>::AddVariable(Variable<T> *variable)
{
    variables.push_back(variable);
    variable->AddConstraint(this);
}

template <class T>
void Constraint<T>::AddVariable(int num_variables, ...)
{
    va_list ap;
    va_start(ap, num_variables);

    for (int i = 0; i < num_variables; i++) {
        Variable<T> *variable = va_arg(ap, Variable<T> *);
        AddVariable(variable);
    }

    va_end(ap);
}

template <class T>
void Constraint<T>::UpdateBounds()
{
    for (unsigned i = 0; i < variables.size(); i++) {
        T  low, high;
        variables[i]->GetDomain().GetBounds(low, high);
        if (i == 0) {
            low_value  = low;
            high_value = high;
        } else {
            if (low_value > low)
                low_value = low;
            if (high_value < high)
                high_value = high;
        }
    }
}

template <class T>
void Constraint<T>::GetBounds(T &low, T &high) const
{
    low  = low_value;
    high = high_value;
}

#include "Problem.h"

template <class T>
bool Constraint<T>::OnDecided(Variable<T> *decided)
{
#ifdef VERBOSE
    problem->ShowState(decided);
#endif
    return true;
}

#endif
