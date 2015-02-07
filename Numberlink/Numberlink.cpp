#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <vector>
using namespace std;

#include "Problem.h"

class Link : public Constraint<int> {
 public:
  Link(int same_count) : same_count(same_count) {}
  bool Enforce() override;

 private:
  const int same_count;
};

bool Link::Enforce() {
  if (variables[0]->GetDomainSize() > 1) return true;
  int a = variables[0]->GetValue(0);

  int num_decided_same = 0, num_maybe_same = 0;
  Variable<int> *maybe_variables[variables.size()];
  for (size_t i = 1; i < variables.size(); ++i) {
    if (variables[i]->GetDomainSize() == 1 && variables[i]->GetValue(0) == a)
      ++num_decided_same;
    else if (variables[i]->GetDomainSize() > 1 &&
             variables[i]->GetDomain().Contains(a))
      maybe_variables[num_maybe_same++] = variables[i];
  }
  if (num_decided_same > same_count) return false;
  if (num_decided_same == same_count) {
    for (int i = 0; i < num_maybe_same; ++i) {
      maybe_variables[i]->Exclude(a);
#if 0
      // Too slow to start propagation again.
      if (maybe_variables[i]->GetDomainSize() == 1)
        maybe_variables[i]->PropagateDecision(this);
#endif
    }
    return true;
  }
  if (num_decided_same + num_maybe_same < same_count) return false;
  if (num_decided_same + num_maybe_same > same_count) return true;
  for (int i = 0; i < num_maybe_same; ++i) {
    maybe_variables[i]->Decide(a);
    maybe_variables[i]->PropagateDecision(this);
  }
  return true;
}

class Numberlink : public Problem<int> {
 public:
  Numberlink(const Option &option);
  void ShowState(Variable<int> *current) override;
  void ShowSolution() override;

 private:
  int rows, columns;
  vector<vector<Variable<int> *>> grid;
};

Numberlink::Numberlink(const Option &option) : Problem<int>(option) {
  int num_items = scanf("%d %d", &rows, &columns);
  assert(num_items == 2);
  int values[rows][columns];
  int min_value = INT_MAX, max_value = 0;
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < columns; ++c) {
      int num_items = scanf("%d", &values[r][c]);
      assert(num_items == 1);
      if (values[r][c] > 0) {
        min_value = min(min_value, values[r][c]);
        max_value = max(max_value, values[r][c]);
      }
    }

  grid.resize(rows);
  for (int r = 0; r < rows; ++r) grid[r].resize(columns);

  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < columns; ++c) {
      if (values[r][c])
        grid[r][c] = new Variable<int>(values[r][c], values[r][c]);
      else
        grid[r][c] = new Variable<int>(min_value, max_value);
    }
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < columns; ++c) {
      auto link = new Link(values[r][c] > 0 ? 1 : 2);
      // Add center
      link->AddVariable(grid[r][c]);
      // Add neighbors
      if (r > 0) link->AddVariable(grid[r - 1][c]);
      if (c > 0) link->AddVariable(grid[r][c - 1]);
      if (r < rows - 1) link->AddVariable(grid[r + 1][c]);
      if (c < columns - 1) link->AddVariable(grid[r][c + 1]);
      AddConstraint(link);
    }
}

void Numberlink::ShowState(Variable<int> *current) {
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      if (grid[r][c]->GetDomainSize() == 1)
        printf("%3d", grid[r][c]->GetValue(0));
      else
        printf("   ");
    }
    putchar('\n');
  }
}

void Numberlink::ShowSolution() {
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) printf("%3d", grid[r][c]->GetValue(0));
    putchar('\n');
  }
}

int main(int argc, char *argv[]) {
  Option option;
  option.sort = Option::SORT_DOMAIN_SIZE;
  option.arc_consistency = true;
  option.GetOptions(argc, argv);

  Numberlink puzzle(option);
  puzzle.Solve();

  return 0;
}

