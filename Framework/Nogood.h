#ifndef NOGOOD_H
#define NOGOOD_H

#include "Constraint.h"

template <class T>
class Nogood : public Constraint<T> {
 public:
  Nogood(const vector<pair<Variable<T>*, T>>& assignments);
  ~Nogood();
  bool OnDecided(Variable<T>* decided);

 private:
  vector<T> values;
  size_t decisions = 0;
};

template <class T>
Nogood<T>::Nogood(const vector<pair<Variable<T>*, T>>& assignments) {
  vector<Variable<T>*>& variables = Constraint<T>::variables;
  variables.reserve(assignments.size());
  values.reserve(assignments.size());
  for (const auto& assignment : assignments) {
    Constraint<T>::AddVariable(assignment.first);
    values.push_back(assignment.second);
  }
}
template <class T>
Nogood<T>::~Nogood() {
  vector<Variable<T>*>& variables = Constraint<T>::variables;
  printf("%lu: ", decisions);
  for (size_t i = 0; i < values.size(); ++i)
    printf("%lu=%d ", variables[i]->GetId(), (int)values[i]);
  putchar('\n');
}

template <class T>
bool Nogood<T>::OnDecided(Variable<T>* decided) {
  vector<Variable<T>*>& variables = Constraint<T>::variables;
  size_t num_variables = variables.size();
  size_t num_decided = 0;
  size_t undecided_index = num_variables;

  for (size_t i = 0; i < num_variables; i++) {
    if (variables[i]->GetDomainSize() == 1) {
      if (variables[i]->GetValue(0) == values[i])
        num_decided++;
      else
        return true;
    } else {
      if (undecided_index == num_variables)
        undecided_index = i;
      else
        return true;
    }
  }

  if (num_decided < num_variables - 1) return true;
  if (num_decided == num_variables) return false;

  // Only one variable is undecided. Apply the constraint.
  Variable<T>* undecided = variables[undecided_index];
  undecided->Exclude(values[undecided_index]);
  switch (undecided->GetDomainSize()) {
    case 0:
      return false;
    // One constraint can only decide at most one variable,
    // so exclude it from propagation.
    case 1:
      decisions++;
      return undecided->PropagateDecision(this);
    default:
      return true;
  }
}

#endif
