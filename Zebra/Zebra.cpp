#include <stdio.h>
#include <stdlib.h>

#include "OneToOne.h"
#include "Same.h"
#include "Function.h"
#include "Problem.h"

class Zebra : public Problem<int> {
 public:
  Zebra(Option option);
  void ShowSolution();

 private:
  Variable<int> *v[25];

  static const char *attr_names[5];
  static const char *value_names[25];

  enum { Color, Nationality, Drink, Smoke, Pet };

  enum {
    Blue,
    Green,
    Ivory,
    Red,
    Yellow,
    Englishman,
    Japanese,
    Norwegian,
    Spaniard,
    Ukrainian,
    Coffee,
    OrangeJuice,
    Milk,
    Tea,
    Water,
    Chesterfield,
    Kools,
    LuckyStrike,
    OldGold,
    Parliament,
    Dog,
    Fox,
    Horse,
    Snails,
    Zebra1
  };

  struct Next {
    bool operator()(int count, const int values[], int) {
      return abs(values[0] - values[1]) == 1;
    }
  };

  struct RightNext {
    bool operator()(int count, const int values[], int) {
      return values[0] - values[1] == 1;
    }
  };
};

const char *Zebra::attr_names[5] = {"Color", "Nationality", "Drink", "Smoke",
                                    "Pet"};

const char *Zebra::value_names[25] = {
    "Blue",         "Green",       "Ivory",       "Red",      "Yellow",
    "Englishman",   "Japanese",    "Norwegian",   "Spaniard", "Ukrainian",
    "Coffee",       "OrangeJuice", "Milk",        "Tea",      "Water",
    "Chesterfield", "Kools",       "LuckyStrike", "OldGold",  "Parliament",
    "Dog",          "Fox",         "Horse",       "Snails",   "Zebra"};

Zebra::Zebra(Option option) : Problem<int>(option) {
  for (int attr = 0; attr < 25; attr++) {
    v[attr] = new Variable<int>(1, 5);
    v[attr]->SetName(value_names[attr]);
  }

  OneToOne<int> *color = new OneToOne<int>();
  OneToOne<int> *nationality = new OneToOne<int>();
  OneToOne<int> *drink = new OneToOne<int>();
  OneToOne<int> *smoke = new OneToOne<int>();
  OneToOne<int> *pet = new OneToOne<int>();
  for (int i = 0; i < 5; i++) {
    color->AddVariable(v[Blue + i]);
    nationality->AddVariable(v[Englishman + i]);
    drink->AddVariable(v[Coffee + i]);
    smoke->AddVariable(v[Chesterfield + i]);
    pet->AddVariable(v[Dog + i]);
  }
  AddConstraint(color);
  AddConstraint(nationality);
  AddConstraint(drink);
  AddConstraint(smoke);
  AddConstraint(pet);

  // The Englishman lives in the red house.
  Same<int> *c1 = new Same<int>();
  c1->AddVariable(2, v[Englishman], v[Red]);
  AddConstraint(c1);

  // The Spaniard owns the dog.
  Same<int> *c2 = new Same<int>();
  c2->AddVariable(2, v[Spaniard], v[Dog]);
  AddConstraint(c2);

  // Coffee is drunk in the green house.
  Same<int> *c3 = new Same<int>();
  c3->AddVariable(2, v[Coffee], v[Green]);
  AddConstraint(c3);

  // The Ukrainian drinks tea.
  Same<int> *c4 = new Same<int>();
  c4->AddVariable(2, v[Ukrainian], v[Tea]);
  AddConstraint(c4);

  // The green house is immediately to the right of the ivory house.
  Function<int, RightNext> *c5 = new Function<int, RightNext>();
  c5->AddVariable(2, v[Green], v[Ivory]);
  AddConstraint(c5);

  // The Old Gold smoker owns snails.
  Same<int> *c6 = new Same<int>();
  c6->AddVariable(2, v[OldGold], v[Snails]);
  AddConstraint(c6);

  // Kools are smoked in the yellow house.
  Same<int> *c7 = new Same<int>();
  c7->AddVariable(2, v[Kools], v[Yellow]);
  AddConstraint(c7);

  // Milk is drunk in the middle house.
  v[Milk]->Decide(3);

  // The Norwegian lives in the first house.
  v[Norwegian]->Decide(1);

  // The man who smokes Chesterfields lives in the house next to the man with
  // the fox.
  Function<int, Next> *c8 = new Function<int, Next>();
  c8->AddVariable(2, v[Chesterfield], v[Fox]);
  AddConstraint(c8);

  // Kools are smoked in the house next to the house where the horse is kept.
  Function<int, Next> *c9 = new Function<int, Next>();
  c9->AddVariable(2, v[Kools], v[Horse]);
  AddConstraint(c9);

  // The Lucky Strike smoker drinks orange juice.
  Same<int> *c10 = new Same<int>();
  c10->AddVariable(2, v[LuckyStrike], v[OrangeJuice]);
  AddConstraint(c10);

  // The Japanese smokes Parliaments.
  Same<int> *c11 = new Same<int>();
  c11->AddVariable(2, v[Japanese], v[Parliament]);
  AddConstraint(c11);

  // The Norwegian lives next to the blue house.
  Function<int, Next> *c12 = new Function<int, Next>();
  c12->AddVariable(2, v[Norwegian], v[Blue]);
  AddConstraint(c12);
}

void Zebra::ShowSolution() {
  for (int attr = 0; attr < 5; attr++) {
    // printf("%12s ", attr_names[attr]);
    Variable<int> *variables[6];
    for (int i = 0; i < 5; i++)
      variables[v[attr * 5 + i]->GetValue(0)] = v[attr * 5 + i];
    for (int house = 1; house <= 5; house++)
      printf("%12s=%d ", variables[house]->GetName(), house);
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  Option option;
  option.GetOptions(argc, argv);

  Zebra puzzle(option);
  puzzle.Solve();

  return 0;
}
