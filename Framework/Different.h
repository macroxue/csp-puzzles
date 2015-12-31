#ifndef DIFFERENT_H
#define DIFFERENT_H

#include "Constraint.h"

//
// Different: All variables must take different values.
//
template <class T>
class Different : public Constraint<T> {
 public:
  bool OnDecided(Variable<T> *decided);

 private:
};

template <class T>
bool Different<T>::OnDecided(Variable<T> *decided) {
  assert(decided->GetDomainSize() == 1);

  // Remove the used value from other variables' domains
  vector<Variable<T> *> &variables = Constraint<T>::variables;
  size_t num_variables = variables.size();
  size_t value = decided->GetValue(0);

  for (size_t i = 0; i < num_variables; i++) {
    if (variables[i] == decided) continue;

    // One constraint can decide multiple variables,
    // so include it in the propagation.
    if (!variables[i]->Exclude(value, NULL)) return false;
  }
  return true;
}

#endif
