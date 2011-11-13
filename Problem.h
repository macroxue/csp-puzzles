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
        void GetOptions(int argc, char *argv[]);
        void AddConstraint(int num_variables, ...);
        void AddConstraint(Constraint<T> *constraint);
        void ActivateConstraint(Constraint<T> *constraint);
        void Solve();
        virtual void ShowState(Variable<T> *current);
        virtual void ShowSolution();

    private:
        void AddVariable(Variable<T> *variable);
        bool EnforceActiveConstraints(bool consistent);
        bool EnforceArcConsistency(unsigned v);
        void Search(unsigned v);
        void Sort(unsigned v);
        void StartCheckpoint();
        void RestoreCheckpoint();

        vector<Variable<T> *>    variables;
        vector<Constraint<T> *>  constraints;
        Queue<Constraint<T> *>   active_constraints;

        typedef vector< pair<Variable<T> *, int> > Storage;
        Storage                  storage;

    protected:
        int                      num_solutions;
        long                     search_count;

        Option                   option;
};

template <class T>
Problem<T>::Problem(Option option)
    : num_solutions(0), search_count(0), option(option)
{
}

template <class T>
void Problem<T>::AddConstraint(Constraint<T> *constraint)
{
    constraints.push_back(constraint);
    for (unsigned i = 0; i < constraint->variables.size(); i++) {
        Variable<T> *variable = constraint->variables[i];
        if (find(variables.begin(), variables.end(), variable) == variables.end())
            AddVariable(variable);
    }

    constraint->problem = this;
}

template <class T>
void Problem<T>::ActivateConstraint(Constraint<T> *constraint)
{
    if (constraint->IsActive())
        return;

    constraint->SetActive(true);
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
        constraint->SetActive(false);
        if (consistent)
            consistent = constraint->Enforce();
    }
    return consistent;
}

template <class T>
bool Problem<T>::EnforceArcConsistency(unsigned v)
{
    for (unsigned i = v; i < variables.size(); i++) {
        Variable<T> *variable = variables[i];
        if (variables[i]->GetDomainSize() == 1)
            continue;

        for (int j = 0; j < variables[i]->GetDomainSize(); j++) {
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
            }
        }
    }
    return true;
}

template <class T>
void Problem<T>::Solve()
{
    for (unsigned i = 0; i < constraints.size(); i++) {
        constraints[i]->UpdateBounds();
        ActivateConstraint(constraints[i]);
    }

    for (unsigned i = 0; i < variables.size(); i++) {
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

    if (option.arc_consistency && !EnforceArcConsistency(0))
        return;

    storage.clear();

    for (unsigned i = 0; i < constraints.size(); i++)
        constraints[i]->UpdateBounds();

    Sort(0);
    DEBUG( ShowState(NULL) );
    Search(0);
}

template <class T>
void Problem<T>::Search(unsigned v)
{
    search_count++;

    // Skip variables that have been decided.
    while (v < variables.size() && variables[v]->GetDomainSize() == 1) v++;
    if (v == variables.size()) {
        num_solutions++;
        printf("----- Solution %d after %ld searches -----\n",
                num_solutions, search_count);
        ShowSolution();
        return;
    }

    Variable<T> *variable = variables[v];
    DEBUG( variable->ShowDomain() );

    while (variable->GetDomainSize() > 0) {
        StartCheckpoint();
        T value = variable->GetValue(0);
        DEBUG( printf("Value %d\n", value) );

        variable->Decide(value);
        bool consistent = variable->PropagateDecision(NULL);
        consistent = EnforceActiveConstraints(consistent);
        if (consistent) {
            Sort(v + 1);
            DEBUG( ShowState(variable) );
            Search(v + 1);
        } else {
            variable->failures++;
        }
        RestoreCheckpoint();
        variable->Exclude(value);
    }
}

template <class T>
void Problem<T>::Sort(unsigned v)
{
    switch (option.sort) {
        case Option::SORT_DISABLED:
            break;
        case Option::SORT_DOMAIN_SIZE:
            for (unsigned i = v+1; i < variables.size(); i++) {
                int domain_size = variables[i]->GetDomainSize();
                if (domain_size == 1) {
                    swap(variables[v], variables[i]);
                    v++;
                } else if (variables[v]->GetDomainSize() > domain_size)
                    swap(variables[v], variables[i]);
            }
            break;
        case Option::SORT_FAILURES:
            for (unsigned i = v+1; i < variables.size(); i++) 
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
        pair<Variable<T> *, int> &state = storage.back();
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
    for (unsigned i = 0; i < variables.size(); i++) {
        printf("[ ");
        for (int v = 0; v < variables[i]->GetDomainSize(); v++) 
            printf("%d ", variables[i]->GetValue(v));
        printf("]");
        putchar(variables[i] == current ? '*' : ' ');
    }
    putchar('\n');
}

template <class T>
void Problem<T>::ShowSolution()
{
    printf("----- Solution %d -----\n", num_solutions);
    ShowState(NULL);
}

#endif
