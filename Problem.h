#ifndef PROBLEM_H
#define PROBLEM_H

#include "Constraint.h"

#include <algorithm>
#include <typeinfo>
#include <vector>
using namespace std;

//
// A problem has a set of variables and a set of constraints.
//
template <class T>
class Problem 
{
    public:
        Problem();
        void AddConstraint(int num_variables, ...);
        void AddConstraint(Constraint<T> *constraint);
        void Solve(bool sort);
        virtual void ShowState(Variable<T> *current);
        virtual void ShowSolution();

    private:
        void AddVariable(Variable<T> *variable);
        void Search(unsigned v);
        void Sort(unsigned v);
        void StartCheckpoint();
        void RestoreCheckpoint();

        vector<Variable<T> *>    variables;
        vector<Constraint<T> *>  constraints;

        typedef vector< pair<Variable<T> *, int> > Storage;
        Storage                  storage;

        bool                     to_sort;

    protected:
        int                      num_solutions;
        long                     search_count;
};

template <class T>
Problem<T>::Problem()
    : num_solutions(0), search_count(0)
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
}

template <class T>
void Problem<T>::AddVariable(Variable<T> *variable)
{
    variables.push_back(variable);
    variable->SetStorage(&storage);
}

template <class T>
void Problem<T>::Solve(bool sort)
{
    to_sort = sort;

    for (unsigned i = 0; i < constraints.size(); i++)
        constraints[i]->UpdateBounds();

    for (unsigned i = 0; i < variables.size(); i++) {
        if (variables[i]->GetDomainSize() == 1) {
            bool consistent = variables[i]->PropagateDecision(NULL);
            if (!consistent)
                return;
            variables.erase(variables.begin() + i);
            i--;
        }
    }
    storage.clear();

    for (unsigned i = 0; i < constraints.size(); i++)
        constraints[i]->UpdateBounds();

#ifdef VERBOSE
    ShowState(NULL);
#endif
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

#ifdef VERBOSE
    printf("Search var %d = ", v);
    for (int i = 0; i < variable->GetDomainSize(); i++)
        printf("%d ", variable->GetValue(i));
    printf("\n");
    int i = 0;
#endif

    while (variable->GetDomainSize() > 0) {
        StartCheckpoint();
        T value = variable->GetValue(0);
#ifdef VERBOSE
        printf("Search var %d [%d] = %d\n", v, i++, value);
#endif
        variable->Decide(value);
        bool consistent = variable->PropagateDecision(NULL);
        if (consistent) {
            if (to_sort)
                Sort(v + 1);
#ifdef VERBOSE
            ShowState(variable);
#endif
            Search(v + 1);
        }
        RestoreCheckpoint();
        variable->Exclude(value);
    }
}

template <class T>
struct compare_domain_size
{
    bool operator ()(Variable<T> *v1, Variable<T> *v2)
    {
        return v1->GetDomainSize() < v2->GetDomainSize();
    }
};

template <class T>
void Problem<T>::Sort(unsigned v)
{
    compare_domain_size<T>  compare;
    sort(variables.begin() + v, variables.end(), compare);
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
