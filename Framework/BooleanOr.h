#ifndef BOOLEAN_OR_H
#define BOOLEAN_OR_H

#include "Constraint.h"

//
// BooleanOr: x1 | ~x2 | x3 | ... = true
//
class BooleanOr : public Constraint<bool> {
 public:
  BooleanOr(const vector<bool> is_positive);
  bool OnDecided(Variable<bool> *decided);

 private:
  const vector<bool> is_positive;
};

BooleanOr::BooleanOr(const vector<bool> is_positive)
    : is_positive(is_positive) {}

bool BooleanOr::OnDecided(Variable<bool> *decided) {
  vector<Variable<bool> *> &variables = Constraint<bool>::variables;
  size_t num_variables = variables.size();
  size_t num_decided = 0;
  size_t undecided_index = 0;

  bool result = false;
  for (size_t i = 0; i < num_variables; i++) {
    if (variables[i]->GetDomainSize() == 1) {
      num_decided++;
      bool value = variables[i]->GetValue(0);
      result |= is_positive[i] ? value : !value;
      if (result) return true;
    } else {
      undecided_index = i;
    }
  }

  if (num_decided < num_variables - 1) return true;
  if (num_decided == num_variables) {
    // TODO: Why is this needed? Should be true always.
    return result == true;
  }

  // Only one variable is undecided. Apply the constraint.
  Variable<bool> *undecided = variables[undecided_index];
  bool target_value = is_positive[undecided_index] ? true : false;

  // Target value may not be in the domain. Need to check.
  undecided->Decide(target_value);
  if (undecided->GetDomainSize() == 0) return false;

  // One constraint can only decide at most one variable,
  // so exclude it from propagation.
  return undecided->PropagateDecision(this);
}

#endif
