#include <stdio.h>
#include <stdlib.h>

#include "BooleanOr.h"
#include "Problem.h"

const int MAX_VARS = 100;
class Sat : public Problem<bool> {
 public:
  Sat(Option option);
  void ShowSolution();

 private:
  vector<Variable<bool> *> v;
};

Sat::Sat(Option option) : Problem<bool>(option) {
  int index;
  vector<int> indices;
  vector<bool> is_positive;
  while (scanf("%d ", &index) == 1) {
    if (index == 0) {
      auto c = new BooleanOr(is_positive);
      for (auto i : indices) c->AddVariable(v[i]);
      AddConstraint(c);
      indices.clear();
      is_positive.clear();
    } else {
      size_t i = abs(index);
      indices.push_back(i);
      is_positive.push_back(index > 0);
      if (v.size() < i + 1) v.resize(i + 1);
      if (!v[i]) v[i] = new Variable<bool>(false, true);
    }
  }
}

void Sat::ShowSolution() {
  for (size_t i = 0; i < v.size(); ++i)
    if (v[i] && v[i]->GetValue(0)) printf("%lu ", i);
  puts("");
}

int main(int argc, char *argv[]) {
  Option option;
  option.arc_consistency = true;
  option.sort = Option::SORT_FAILURES;
  option.GetOptions(argc, argv);

  Sat sat(option);
  sat.Solve();

  return 0;
}
