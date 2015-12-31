#ifndef SAME_H
#define SAME_H

#include "Constraint.h"

//
// Same: All variables must take the same value.
//
template <class T>
class Same : public Constraint<T> {
 public:
  bool OnDecided(Variable<T> *decided);

 private:
};

template <class T>
bool Same<T>::OnDecided(Variable<T> *decided) {
  assert(decided->GetDomainSize() == 1);

  // Apply the same value to other variables' domains
  vector<Variable<T> *> &variables = Constraint<T>::variables;
  size_t num_variables = variables.size();
  size_t value = decided->GetValue(0);

  for (size_t i = 0; i < num_variables; i++) {
    if (variables[i] == decided) continue;

    // One constraint can decide multiple variables,
    // so include it in the propagation.
    if (!variables[i]->Decide(value, NULL)) return false;
  }
  return true;
}

#endif
