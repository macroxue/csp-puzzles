#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "Queue.h"

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
class Constraint : public QueueObject {
 public:
  Constraint();

  void AddVariable(Variable<T> *variable);
  void AddVariable(size_t num_variables, ...);

  void SetProblem(Problem<T> *the_problem);
  void UpdateBounds();
  void GetBounds(T &low, T &high) const;

  // Enforce the constraint.
  // The domains can be reduced to meet the constraint.
  // False is returned if the constraint cannot be met.
  virtual bool OnDecided(Variable<T> *decided);
  virtual bool OnReduced(Variable<T> *reduced) { return true; }
  virtual bool Enforce() { return true; }

  void ActivateVariables();

 protected:
  vector<Variable<T> *> variables;
  Problem<T> *problem;
  T low_value;
  T high_value;
};

#include "Variable.h"

#include <stdarg.h>

template <class T>
Constraint<T>::Constraint() {}

template <class T>
void Constraint<T>::AddVariable(Variable<T> *variable) {
  variables.push_back(variable);
  variable->AddConstraint(this);
}

template <class T>
void Constraint<T>::AddVariable(size_t num_variables, ...) {
  va_list ap;
  va_start(ap, num_variables);

  for (size_t i = 0; i < num_variables; i++) {
    Variable<T> *variable = va_arg(ap, Variable<T> *);
    AddVariable(variable);
  }

  va_end(ap);
}

template <class T>
void Constraint<T>::SetProblem(Problem<T> *the_problem) {
  problem = the_problem;
  for (size_t i = 0; i < variables.size(); i++) {
    Variable<T> *variable = variables[i];
    problem->AddVariable(variable);
  }
}

template <class T>
void Constraint<T>::UpdateBounds() {
  for (size_t i = 0; i < variables.size(); i++) {
    T low, high;
    variables[i]->GetDomain().GetBounds(low, high);
    if (i == 0) {
      low_value = low;
      high_value = high;
    } else {
      if (low_value > low) low_value = low;
      if (high_value < high) high_value = high;
    }
  }
}

template <class T>
void Constraint<T>::GetBounds(T &low, T &high) const {
  low = low_value;
  high = high_value;
}

template <class T>
bool Constraint<T>::OnDecided(Variable<T> *decided) {
  problem->ActivateConstraint(this);
  return true;
}

template <class T>
void Constraint<T>::ActivateVariables() {
  for (size_t i = 0; i < variables.size(); i++) variables[i]->active = true;
}

#endif
