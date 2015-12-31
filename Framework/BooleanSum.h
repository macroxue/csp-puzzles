#ifndef BOOLEAN_SUM_H
#define BOOLEAN_SUM_H

#include "Constraint.h"

enum Relation { EQUAL_TO, LESS_THAN, GREATER_THAN };

//
// BooleanSum: x1 + x2 + x3 + ... <Relation> target
//
template <class T>
class BooleanSum : public Constraint<T> {
 public:
  BooleanSum(Relation relation, size_t target);
  bool Enforce() override;

 private:
  Relation relation;
  size_t target;
};

template <class T>
BooleanSum<T>::BooleanSum(Relation relation, size_t target)
    : relation(relation), target(target) {}

template <class T>
bool BooleanSum<T>::Enforce() {
  vector<Variable<T> *> &variables = Constraint<T>::variables;
  size_t num_variables = variables.size();
  size_t num_decided = 0;

  int sum = 0;
  for (size_t i = 0; i < num_variables; i++) {
    if (variables[i]->GetDomainSize() == 1) {
      num_decided++;
      sum += variables[i]->GetValue(0);
      if ((relation == EQUAL_TO && sum > target) ||
          (relation == LESS_THAN && sum >= target))
        return false;
    }
  }

  size_t num_undecided = num_variables - num_decided;
  if ((relation == EQUAL_TO && sum + num_undecided < target) ||
      (relation == GREATER_THAN && sum + num_undecided <= target))
    return false;

  if (num_undecided == 0) return true;
  if ((relation == EQUAL_TO && sum == target) ||
      (relation == LESS_THAN && sum == target - 1)) {
    for (size_t i = 0; i < num_variables; i++) {
      if (variables[i]->GetDomainSize() > 1) {
        if (!variables[i]->Decide(false, this)) return false;
      }
    }
  } else if ((relation == EQUAL_TO && sum + num_undecided == target) ||
             (relation == GREATER_THAN && sum + num_undecided == target + 1)) {
    for (size_t i = 0; i < num_variables; i++) {
      if (variables[i]->GetDomainSize() > 1) {
        if (!variables[i]->Decide(true, this)) return false;
      }
    }
  }
  return true;
}

#endif
