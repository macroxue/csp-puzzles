#ifndef OPTION_H
#define OPTION_H

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Option {
  bool arc_consistency = false;
  bool debug = false;
  bool interactive = false;
  bool learning = false;
  size_t num_solutions = 2;
  bool optimize = false;
  float decay = 0.99;
  size_t restart = INT_MAX;
  enum sort_t { SORT_DISABLED, SORT_DOMAIN_SIZE, SORT_FAILURES, SORT_WEIGHT };
  sort_t sort = SORT_DISABLED;
  enum sort_values_t {
    SORT_VALUES_DISABLED,
    SORT_VALUES_ASCENDING,
    SORT_VALUES_DESCENDING,
    SORT_VALUES_IN_CONSTRAINT,
  };
  sort_values_t sort_values = SORT_VALUES_DISABLED;

  void GetOptions(int argc, char *argv[]);
  void ShowOptions() const;
};

void Option::GetOptions(int argc, char *argv[]) {
  int c;
  while ((c = getopt(argc, argv, "adln:or:s:v:y:")) != -1) {
    switch (c) {
      case 'a':
        arc_consistency = !arc_consistency;
        break;
      case 'd':
        debug = !debug;
        break;
      case 'l':
        learning = !learning;
        break;
      case 'n':
        num_solutions = atoi(optarg);
        break;
      case 'o':
        optimize = !optimize;
        break;
      case 'r':
        restart = atoi(optarg);
        break;
      case 's':
        switch (optarg[0]) {
          case 'd':
            sort = SORT_DOMAIN_SIZE;
            break;
          case 'f':
            sort = SORT_FAILURES;
            break;
          case 'w':
            sort = SORT_WEIGHT;
            break;
          case 'D':
            sort = SORT_DISABLED;
            break;
          default:
            printf("Invalid variable-ordering option: %s\n", optarg);
            exit(1);
        }
        break;
      case 'v':
        switch (optarg[0]) {
          case 'a':
            sort_values = SORT_VALUES_ASCENDING;
            break;
          case 'c':
            sort_values = SORT_VALUES_IN_CONSTRAINT;
            break;
          case 'd':
            sort_values = SORT_VALUES_DESCENDING;
            break;
          case 'D':
            sort_values = SORT_VALUES_DISABLED;
            break;
          default:
            printf("Invalid value-ordering option: %s\n", optarg);
            exit(1);
        }
        break;
      case 'y':
        decay = atof(optarg);
        break;
    }
  }
}

void Option::ShowOptions() const {
  printf("arc_consistency = %d\n", arc_consistency);
  printf("debug = %d\n", debug);
  printf("interactive = %d\n", interactive);
  printf("learning = %d\n", learning);
  printf("num_solutions = %ld\n", num_solutions);
  printf("optimize = %d\n", optimize);
  printf("decay = %f\n", decay);
  printf("restart = %ld\n", restart);
  printf("sort = %d\n", sort);
  printf("sort_values = %d\n", sort_values);
}

#endif
