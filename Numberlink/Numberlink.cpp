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

  void AddDiagonal(Variable<int> *diagonal) { diagonals.push_back(diagonal); }

 private:
  bool CheckDiagonals() const;

  const int same_count;
  vector<Variable<int> *> diagonals;
};

bool Link::Enforce() {
  if (variables[0]->GetDomainSize() > 1) return true;
  int a = variables[0]->GetValue(0);

  int num_decided_same = 0, num_maybe_same = 0;
  Variable<int> *maybe_variables[variables.size()];
  for (size_t i = 1; i < variables.size(); ++i) {
    if (variables[i]->Is(a))
      ++num_decided_same;
    else if (variables[i]->Maybe(a))
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
    return CheckDiagonals();
  }
  if (num_decided_same + num_maybe_same < same_count) return false;
  if (num_decided_same + num_maybe_same > same_count) return true;
  for (int i = 0; i < num_maybe_same; ++i) {
    maybe_variables[i]->Decide(a);
    maybe_variables[i]->PropagateDecision(this);
  }
  return CheckDiagonals();
}

bool Link::CheckDiagonals() const {
  if (variables.size() != 5) return true;
  int a = variables[0]->GetValue(0);
  for (size_t i = 0; i < diagonals.size() - 1; ++i) {
    if (variables[i + 1]->Is(a) && variables[i + 2]->Is(a)) {
      diagonals[i]->Exclude(a);
      return diagonals[i]->GetDomainSize() > 0;
    }
  }
  if (variables.back()->Is(a) && variables[1]->Is(a)) {
    diagonals.back()->Exclude(a);
    return diagonals.back()->GetDomainSize() > 0;
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
  vector<vector<int>> values;
  vector<vector<Variable<int> *>> grid;
};

Numberlink::Numberlink(const Option &option) : Problem<int>(option) {
  int num_items = scanf("%d %d", &rows, &columns);
  assert(num_items == 2);
  values.resize(rows);
  for (int r = 0; r < rows; ++r) values[r].resize(columns);
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
      // Add neighbors clockwise.
      if (r > 0) link->AddVariable(grid[r - 1][c]);
      if (c < columns - 1) link->AddVariable(grid[r][c + 1]);
      if (r < rows - 1) link->AddVariable(grid[r + 1][c]);
      if (c > 0) link->AddVariable(grid[r][c - 1]);
      // Add diagonals clockwise, ignoring 2 or 3 neighbors for simplicity
      if (r > 0 && r < rows - 1 && c > 0 && c < columns - 1) {
        link->AddDiagonal(grid[r - 1][c + 1]);
        link->AddDiagonal(grid[r + 1][c + 1]);
        link->AddDiagonal(grid[r + 1][c - 1]);
        link->AddDiagonal(grid[r - 1][c - 1]);
      }
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
    for (int c = 0; c < columns; ++c) {
      int v = grid[r][c]->GetValue(0);
      bool same_left = c > 0 && grid[r][c - 1]->GetValue(0) == v;
      bool same_right = c < columns - 1 && grid[r][c + 1]->GetValue(0) == v;
      if (values[r][c] > 0) {
        if (same_left)
          printf(v >= 10 ? "--%d" : "---%d", v);
        else
          printf("%4d", v);
      } else {
        if (same_left && same_right)
          printf("----");
        else if (same_left)
          printf("---+");
        else if (same_right)
          printf("   +");
        else
          printf("   |");
      }
    }
    putchar('\n');
    if (r < rows - 1) {
      for (int c = 0; c < columns; ++c) {
        int v = grid[r][c]->GetValue(0);
        printf(grid[r + 1][c]->GetValue(0) == v ? "   |" : "    ");
      }
      putchar('\n');
    }
  }
}

int main(int argc, char *argv[]) {
  Option option;
  option.sort = Option::SORT_WEIGHT;
  option.sort_values = Option::SORT_VALUES_DISABLED;
  option.arc_consistency = true;
  option.GetOptions(argc, argv);

  Numberlink puzzle(option);
  puzzle.Solve();

  return 0;
}

