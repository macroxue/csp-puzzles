#ifndef PROBLEM_H
#define PROBLEM_H

#include "Option.h"
#include "Constraint.h"
#include "Queue.h"

#include <algorithm>
#include <typeinfo>
#include <vector>
using namespace std;

#define DEBUG( statement )  if (option.debug) statement

//
// A problem has a set of variables and a set of constraints.
//
template <class T>
class Problem 
{
    public:
        Problem(Option option);
        ~Problem();
        void GetOptions(int argc, char *argv[]);
        void AddConstraint(size_t num_variables, ...);
        void AddConstraint(Constraint<T> *constraint);
        void ActivateConstraint(Constraint<T> *constraint);
        void Solve();
        virtual void ShowState(Variable<T> *current);
        virtual void ShowSolution();

        void IncrementCounter(size_t index, size_t inc = 1);
        virtual void ShowCounters();

    private:
        void AddVariable(Variable<T> *variable);
        bool EnforceActiveConstraints(bool consistent);
        bool EnforceArcConsistency(size_t v);
        void Search(size_t v);
        void Sort(size_t v);
        void StartCheckpoint();
        void RestoreCheckpoint();

        vector<Variable<T> *>    variables;
        vector<Constraint<T> *>  constraints;
        Queue<Constraint<T> *>   active_constraints;

        typedef vector< pair<Variable<T> *, size_t> > Storage;
        Storage                  storage;

    protected:
        size_t                   num_solutions;
        size_t                   search_count;
        size_t                   deadend_count;

        Option                   option;

        double                   start, finish;

        struct Counter {
            const char *         name;
            size_t               value;
            Counter(const char *name = NULL, size_t value = 0)
                : name(name), value(value) {}
        };
        Counter                  counters[16];
};

#include <sys/time.h>

inline double seconds()
{
    struct timeval time_v;
    gettimeofday(&time_v, NULL);
    return time_v.tv_sec + time_v.tv_usec * 1e-6;
}

template <class T>
Problem<T>::Problem(Option option)
    : num_solutions(0), search_count(0), deadend_count(0), option(option)
{
    start = seconds();
}

template <class T>
Problem<T>::~Problem()
{
    if (!option.interactive) {
        finish = seconds();
        printf("Total time: %.3fs\n", finish - start);
    }
}

template <class T>
void Problem<T>::AddConstraint(Constraint<T> *constraint)
{
    constraints.push_back(constraint);
    for (size_t i = 0; i < constraint->variables.size(); i++) {
        Variable<T> *variable = constraint->variables[i];
        if (find(variables.begin(), variables.end(), variable) == variables.end())
            AddVariable(variable);
    }

    constraint->problem = this;
}

template <class T>
void Problem<T>::ActivateConstraint(Constraint<T> *constraint)
{
    if (!constraint->in_queue)
        active_constraints.Enqueue(constraint);
}

template <class T>
void Problem<T>::AddVariable(Variable<T> *variable)
{
    variable->SetId(variables.size());
    variables.push_back(variable);
    variable->SetStorage(&storage);
}

template <class T>
bool Problem<T>::EnforceActiveConstraints(bool consistent)
{
    while (!active_constraints.IsEmpty()) {
        Constraint<T> *constraint = active_constraints.Dequeue();
        if (consistent)
            consistent = constraint->Enforce();
    }
    return consistent;
}

template <class T>
bool Problem<T>::EnforceArcConsistency(size_t v)
{
    bool domain_reduced;
    do {
        domain_reduced = false;
        for (size_t i = v; i < variables.size(); i++) {
            Variable<T> *variable = variables[i];
            if (variable->GetDomainSize() == 1) {
                swap(variables[i], variables[v]);
                v++;
            }
        }

        for (size_t i = v; i < variables.size(); i++) {
            Sort(i);
            Variable<T> *variable = variables[i];
            for (size_t j = 0; j < variable->GetDomainSize(); j++) {
                StartCheckpoint();

                T value = variable->GetValue(j);
                variable->Decide(value);
                bool consistent = variable->PropagateDecision(NULL);
                consistent = EnforceActiveConstraints(consistent);

                RestoreCheckpoint();

                if (!consistent) {
                    variable->Exclude(value);
                    j--;
                    if (variable->GetDomainSize() == 0)
                        return false;
                    domain_reduced = true;
                }
            }
        }
    } while (domain_reduced);
    return true;
}

template <class T>
void Problem<T>::Solve()
{
    for (size_t i = 0; i < constraints.size(); i++) {
        constraints[i]->UpdateBounds();
        ActivateConstraint(constraints[i]);
    }

    for (size_t i = 0; i < variables.size(); i++) {
        if (variables[i]->GetDomainSize() == 1) {
            bool consistent = variables[i]->PropagateDecision(NULL);
            if (!consistent)
                return;
            variables.erase(variables.begin() + i);
            i--;
        }
    }

    if (!EnforceActiveConstraints(true)) 
        return;

    if (option.arc_consistency) {
        bool consistent = EnforceArcConsistency(0);
        printf("Arc consistency time = %.3fs\n", seconds() - start);
        if (!consistent)
            return;
    }

    storage.clear();

    for (size_t i = 0; i < constraints.size(); i++)
        constraints[i]->UpdateBounds();

    Sort(0);
    DEBUG( ShowState(NULL) );

    try {
        Search(0);
    } catch (bool result) {
        ;
    }
    ShowCounters();
}

template <class T>
void Problem<T>::Search(size_t v)
{
    search_count++;

    // Skip variables that have been decided.
    while (v < variables.size() && variables[v]->GetDomainSize() == 1) {
        DEBUG( printf("%ld: Variable %ld = %d, 2\n", v, variables[v]->GetId(), variables[v]->GetValue(0)) );
        v++;
    }
    if (v == variables.size()) {
        num_solutions++;
        printf("----- Solution %ld after %ld searches, %ld deadends and %.3fs -----\n",
                num_solutions, search_count, deadend_count, seconds() - start);
        ShowSolution();
        if (option.num_solutions > 0 && num_solutions >= option.num_solutions) {
            throw true;
        }
        return;
    }

    Variable<T> *variable = variables[v];
    DEBUG( variable->ShowDomain() );

    bool deadend = true;
    while (variable->GetDomainSize() > 0) {
        StartCheckpoint();
        T value = variable->GetValue(0);

        variable->Decide(value);
        bool consistent = variable->PropagateDecision(NULL);
        consistent = EnforceActiveConstraints(consistent);
        if (consistent && option.arc_consistency)
            consistent = EnforceArcConsistency(v+1);
        DEBUG( printf("%ld: Variable %ld = %d, %d\n", v, variable->GetId(), value, consistent) );
        if (consistent) {
            deadend = false;
            Sort(v + 1);
            DEBUG( ShowState(variable) );
            Search(v + 1);
        }
        RestoreCheckpoint();
        variable->Exclude(value);
    }
    if (deadend) {
        deadend_count++;
        variable->failures = deadend_count;
        DEBUG( printf("%ld: Variable %ld deadend %ld\n", v, variable->GetId(), variable->failures) );
    }
}

template <class T>
void Problem<T>::Sort(size_t v)
{
    switch (option.sort) {
        case Option::SORT_DISABLED:
            break;
        case Option::SORT_DOMAIN_SIZE:
            {
                size_t min_domain_size = LONG_MAX, min_index = variables.size();
                for (size_t i = v; i < variables.size(); i++) {
                    size_t domain_size = variables[i]->GetDomainSize();
                    if (domain_size == 1) {
                        swap(variables[v], variables[i]);
                        if (min_index == v)
                            min_index = i;
                        v++;
                    } else if (min_domain_size > domain_size ||
                            (min_domain_size == domain_size &&
                             variables[min_index]->failures < variables[i]->failures)) {
                        min_domain_size = domain_size;
                        min_index = i;
                    }
                }
                if (min_index != variables.size()) 
                    swap(variables[v], variables[min_index]);
                break;
            }
        case Option::SORT_FAILURES:
            {
                size_t max_failures = 0, max_index = variables.size();
                for (size_t i = v; i < variables.size(); i++) {
                    size_t domain_size = variables[i]->GetDomainSize();
                    if (domain_size == 1) {
                        swap(variables[v], variables[i]);
                        if (max_index == v)
                            max_index = i;
                        v++;
                    } else if (max_failures < variables[i]->failures) {
                        max_failures = variables[i]->failures;
                        max_index = i;
                    }
                }
                if (max_index != variables.size()) 
                    swap(variables[v], variables[max_index]);
                break;
            }
    }
}

template <class T>
void Problem<T>::StartCheckpoint()
{
    storage.push_back(make_pair((Variable<T> *)NULL, 0));
}

template <class T>
void Problem<T>::RestoreCheckpoint()
{
    for (;;) {
        pair<Variable<T> *, size_t> &state = storage.back();
        if (state.first) {
            state.first->Restore(state.second);
            storage.pop_back();
        } else {
            storage.pop_back();
            break;
        }
    }
}

template <class T>
void Problem<T>::ShowState(Variable<T> *current)
{
    for (size_t i = 0; i < variables.size(); i++) {
        printf("[ ");
        for (size_t v = 0; v < variables[i]->GetDomainSize(); v++) 
            printf("%d ", variables[i]->GetValue(v));
        printf("]");
        putchar(variables[i] == current ? '*' : ' ');
    }
    putchar('\n');
}

template <class T>
void Problem<T>::ShowSolution()
{
    printf("----- Solution %ld -----\n", num_solutions);
    ShowState(NULL);
}

template <class T>
void Problem<T>::IncrementCounter(size_t index, size_t inc)
{
    counters[index].value += inc;
}

template <class T>
void Problem<T>::ShowCounters()
{
    for (size_t i = 0; counters[i].name; i++)
        printf("%s: %ld\n", counters[i].name, counters[i].value);
}

#endif
