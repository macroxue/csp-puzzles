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

        // Enforce the constraint.
        // The domains can be reduced to meet the constraint.
        // False is returned if the constraint cannot be met.
        virtual bool OnDecided(Variable<T> *decided) = 0;
        virtual bool OnReduced(Variable<T> *reduced) { return true; }

    protected:
        vector<Variable<T> *>  variables;

        friend class Problem<T>;
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

#endif