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

        Option                   option;

        double                   start, finish;
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
    : num_solutions(0), search_count(0), option(option)
{
    start = seconds();
}

template <class T>
Problem<T>::~Problem()
{
    if (!option.interactive) {
        finish = seconds();
        printf("Total time = %.3fs\n", finish - start);
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
}

template <class T>
void Problem<T>::Search(size_t v)
{
    search_count++;

    // Skip variables that have been decided.
    while (v < variables.size() && variables[v]->GetDomainSize() == 1) v++;
    if (v == variables.size()) {
        num_solutions++;
        printf("----- Solution %ld after %ld searches and %.3fs -----\n",
                num_solutions, search_count, seconds() - start);
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
        DEBUG( printf("Value %d\n", value) );

        variable->Decide(value);
        bool consistent = variable->PropagateDecision(NULL);
        consistent = EnforceActiveConstraints(consistent);
        if (consistent) {
            deadend = false;
            Sort(v + 1);
            DEBUG( ShowState(variable) );
            Search(v + 1);
        }
        RestoreCheckpoint();
        variable->Exclude(value);
    }
    variable->failures += deadend;
}

template <class T>
void Problem<T>::Sort(size_t v)
{
    switch (option.sort) {
        case Option::SORT_DISABLED:
            break;
        case Option::SORT_DOMAIN_SIZE:
            for (size_t i = v+1; i < variables.size(); i++) {
                size_t domain_size = variables[i]->GetDomainSize();
                if (domain_size == 1) {
                    swap(variables[v], variables[i]);
                    v++;
                } else if (variables[v]->GetDomainSize() > domain_size)
                    swap(variables[v], variables[i]);
            }
            break;
        case Option::SORT_FAILURES:
            for (size_t i = v+1; i < variables.size(); i++) 
                if (variables[v]->failures < variables[i]->failures)
                    swap(variables[v], variables[i]);
            break;
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

#endif
