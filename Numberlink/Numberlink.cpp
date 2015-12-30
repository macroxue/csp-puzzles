#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>
using namespace std;

#include "BooleanSum.h"
#include "Problem.h"

class Equality : public Constraint<int> {
 public:
  Equality(Variable<int>* n0, Variable<int>* n1, Variable<int>* relation);
  bool Enforce() override;

 private:
  Variable<int>* const n0;
  Variable<int>* const n1;
  Variable<int>* const relation;
};

Equality::Equality(Variable<int>* n0, Variable<int>* n1,
                   Variable<int>* relation)
    : n0(n0), n1(n1), relation(relation) {
  AddVariable(3, n0, n1, relation);
}

bool Equality::Enforce() {
  bool n0_decided = n0->GetDomainSize() == 1;
  bool n1_decided = n1->GetDomainSize() == 1;
  bool relation_decided = relation->GetDomainSize() == 1;

  int v0 = n0->GetValue(0);
  int v1 = n1->GetValue(0);
  int equal = relation->GetValue(0);

  if (n0_decided && n1_decided) {
    if (relation_decided) return equal == (v0 == v1);

    variables[2]->Decide(v0 == v1);
    if (variables[2]->GetDomainSize() == 0) return false;
    return variables[2]->PropagateDecision(this);

  } else if (n0_decided && relation_decided) {
    if (equal)
      variables[1]->Decide(v0);
    else
      variables[1]->Exclude(v0);

    if (variables[1]->GetDomainSize() == 0) return false;
    if (variables[1]->GetDomainSize() == 1)
      return variables[1]->PropagateDecision(this);
    return true;

  } else if (n1_decided && relation_decided) {
    if (equal)
      variables[0]->Decide(v1);
    else
      variables[0]->Exclude(v1);

    if (variables[0]->GetDomainSize() == 0) return false;
    if (variables[0]->GetDomainSize() == 1)
      return variables[0]->PropagateDecision(this);
    return true;

  } else
    return true;
}

class Numberlink : public Problem<int> {
 public:
  Numberlink(const Option& option, bool use_alt_format);
  void ShowState(Variable<int>* current) override;
  bool IsValidSolution() override;
  void ShowSolution() override;

 private:
  int rows, columns;
  vector<vector<int>> values;
  vector<vector<Variable<int>*>> grid;
  vector<vector<Variable<int>*>> horizontal_equal;
  vector<vector<Variable<int>*>> vertical_equal;
};

Numberlink::Numberlink(const Option& option, bool use_alt_format)
    : Problem<int>(option) {
  int num_items = scanf("%d %d", &rows, &columns);
  assert(num_items == 2);
  values.resize(rows);
  for (int r = 0; r < rows; ++r) values[r].resize(columns);
  map<int, int> chars;
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < columns; ++c) {
      int v;
      if (!use_alt_format) {
        int num_items = scanf("%d", &v);
        assert(num_items == 1);
        if (v == 0) continue;
      } else {
        do
          v = getchar();
        while (v != '.' && !isalnum(v));
        if (v == '.') continue;
      }
      auto it = chars.find(v);
      if (it == chars.end()) {
        values[r][c] = chars.size() + 1;
        chars[v] = values[r][c];
      } else {
        values[r][c] = it->second;
      }
    }
  int min_value = 1;
  int max_value = chars.size();

  grid.resize(rows);
  for (int r = 0; r < rows; ++r) {
    grid[r].resize(columns);
    for (int c = 0; c < columns; ++c) {
      if (values[r][c])
        grid[r][c] = new Variable<int>(values[r][c], values[r][c]);
      else
        grid[r][c] = new Variable<int>(min_value, max_value);
    }
  }

  horizontal_equal.resize(rows);
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns - 1; ++c)
      horizontal_equal[r].push_back(new Variable<int>(0, 1));
  }

  vertical_equal.resize(rows - 1);
  for (int r = 0; r < rows - 1; ++r) {
    for (int c = 0; c < columns; ++c)
      vertical_equal[r].push_back(new Variable<int>(0, 1));
  }

  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < columns; ++c) {
      auto sum = new BooleanSum<int>(EQUAL_TO, values[r][c] > 0 ? 1 : 2);
      if (r > 0) sum->AddVariable(vertical_equal[r - 1][c]);
      if (r < rows - 1) sum->AddVariable(vertical_equal[r][c]);
      if (c > 0) sum->AddVariable(horizontal_equal[r][c - 1]);
      if (c < columns - 1) sum->AddVariable(horizontal_equal[r][c]);
      AddConstraint(sum);

      if (c < columns - 1)
        AddConstraint(
            new Equality(grid[r][c], grid[r][c + 1], horizontal_equal[r][c]));

      if (r < rows - 1)
        AddConstraint(
            new Equality(grid[r][c], grid[r + 1][c], vertical_equal[r][c]));

      if (c < columns - 1 && r < rows - 1) {
        // No 2x2 circle of the same numbers.
        auto sum = new BooleanSum<int>(LESS_THAN, 4);
        sum->AddVariable(horizontal_equal[r][c]);
        sum->AddVariable(horizontal_equal[r + 1][c]);
        sum->AddVariable(vertical_equal[r][c]);
        sum->AddVariable(vertical_equal[r][c + 1]);
        AddConstraint(sum);
      }
    }
}

void Numberlink::ShowState(Variable<int>* current) {
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      if (grid[r][c]->GetDomainSize() == 1)
        printf("%2d", grid[r][c]->GetValue(0));
      else
        printf("  ");
      putchar(current == grid[r][c] ? '*' : ' ');

      if (c < columns - 1) {
        if (horizontal_equal[r][c]->GetDomainSize() == 1)
          printf("%2s", horizontal_equal[r][c]->GetValue(0) ? "==" : "!=");
        else
          printf("  ");
        putchar(current == horizontal_equal[r][c] ? '*' : ' ');
      }
    }
    putchar('\n');
    if (r < rows - 1) {
      for (int c = 0; c < columns; ++c) {
        if (vertical_equal[r][c]->GetDomainSize() == 1)
          printf("%2s", vertical_equal[r][c]->GetValue(0) ? "||" : "!!");
        else
          printf("  ");
        putchar(current == vertical_equal[r][c] ? '*' : ' ');
        printf("   ");
      }
      putchar('\n');
    }
  }
}

bool Numberlink::IsValidSolution() {
  // Check if all cells can be visited from starting numbers.
  bool visited[rows][columns];
  memset(visited, false, sizeof(visited));
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      if (visited[r][c] || values[r][c] == 0) continue;
      int a = values[r][c];
      int r1 = r, c1 = c;
      while (true) {
        visited[r1][c1] = true;
        if (r1 > 0 && !visited[r1 - 1][c1] &&
            grid[r1 - 1][c1]->GetValue(0) == a)
          --r1;
        else if (c1 > 0 && !visited[r1][c1 - 1] &&
                 grid[r1][c1 - 1]->GetValue(0) == a)
          --c1;
        else if (r1 < rows - 1 && !visited[r1 + 1][c1] &&
                 grid[r1 + 1][c1]->GetValue(0) == a)
          ++r1;
        else if (c1 < columns - 1 && !visited[r1][c1 + 1] &&
                 grid[r1][c1 + 1]->GetValue(0) == a)
          ++c1;
        else
          break;
      }
    }
  }
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < columns; ++c)
      if (!visited[r][c]) return false;
  return true;
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

int main(int argc, char* argv[]) {
  Option option;
  option.sort = Option::SORT_DOMAIN_SIZE;
  option.sort_values = Option::SORT_VALUES_DESCENDING;
  option.arc_consistency = true;
  option.GetOptions(argc, argv);

  bool use_alt_format = false;
  if (optind < argc) use_alt_format = atoi(argv[optind]) != 0;

  Numberlink puzzle(option, use_alt_format);
  puzzle.Solve();

  return 0;
}

