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

    size_t old_domain_size = variables[i]->GetDomainSize();
    variables[i]->Exclude(value);
    size_t new_domain_size = variables[i]->GetDomainSize();

    if (new_domain_size == 0) return false;
    if (old_domain_size > 1 && new_domain_size == 1) {
      // One constraint can decide multiple variables,
      // so include it in the propagation.
      bool consistent = variables[i]->PropagateDecision(NULL);
      if (!consistent) return false;
    } else if (old_domain_size > new_domain_size) {
      bool consistent = variables[i]->PropagateReduction(NULL);
      if (!consistent) return false;
    }
  }
  return true;
}

#endif
