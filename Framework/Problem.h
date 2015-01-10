#ifndef PROBLEM_H
#define PROBLEM_H

#include "Option.h"
#include "Constraint.h"
#include "Queue.h"

#include <vector>
using namespace std;

#include <sys/time.h>
#include <sys/resource.h>

#define DEBUG( statement )  //if (option.debug) statement

//
// A problem has a set of variables and a set of constraints.
//
template <class T>
class Problem
{
    public:
        Problem(Option option);
        ~Problem();

        double GetTimeUsage();     // in seconds
        size_t GetMemoryUsage();   // in kilo-bytes

        void AddConstraint(size_t num_variables, ...);
        void AddConstraint(Constraint<T> *constraint);
        void ActivateConstraint(Constraint<T> *constraint);
        void AddVariable(Variable<T> *variable);
        void Solve();
        virtual void ShowState(Variable<T> *current);
        virtual void ShowSolution();
        virtual long GetSolutionCost();

        bool CheckSolution(size_t v);
        void ProcessSolution();

        void IncrementCounter(size_t index, size_t inc = 1);
        virtual void ShowCounters();

    private:
        bool EnforceActiveConstraints(bool consistent);
        bool PropagateDecision(Variable<T> *variable);
        void Revise(Variable<T> *variable, size_t v);
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

        struct Counter {
            const char *         name;
            size_t               value;
            Counter(const char *name = NULL, size_t value = 0)
                : name(name), value(value) {}
        };
        Counter                  counters[16];

        long                     min_cost;
};

#include <sys/time.h>

template <class T>
Problem<T>::Problem(Option option)
    : num_solutions(0), search_count(0), deadend_count(0), option(option), min_cost(LONG_MAX)
{
}

template <class T>
Problem<T>::~Problem()
{
    if (!option.interactive) {
        printf("Total time: %.3f s\n", GetTimeUsage());
        printf("Total memory: %ld KB\n", GetMemoryUsage());
    }
}

template <class T>
double Problem<T>::GetTimeUsage()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)
        + (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec)*1e-6;
}

template <class T>
size_t Problem<T>::GetMemoryUsage()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

template <class T>
void Problem<T>::AddConstraint(Constraint<T> *constraint)
{
    constraints.push_back(constraint);
    constraint->SetProblem(this);
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
    if (variable->active)
        return;
    variable->active = true;
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
bool Problem<T>::PropagateDecision(Variable<T> *variable)
{
    bool consistent = variable->PropagateDecision(NULL);
    return EnforceActiveConstraints(consistent);
}

template <class T>
void Problem<T>::Revise(Variable<T> *variable, size_t v)
{
    size_t domain_size = variable->GetDomainSize();
    for (size_t j = 0; j < domain_size; j++) {
        StartCheckpoint();

        T value = variable->GetValue(j);
        variable->Decide(value);
        bool consistent = PropagateDecision(variable);
        if (consistent) {
            DEBUG( printf("Variable %ld = %d is consistent\n",
                        variable->GetId(), value) );
            DEBUG( ShowState(variable) );

            // Take shortcut if a solution is already found
            if (CheckSolution(v))
                consistent = false;
        }

        RestoreCheckpoint();

        if (!consistent) {
            DEBUG( printf("Variable %ld = %d is inconsistent\n",
                        variable->GetId(), value) );
            DEBUG( ShowState(variable) );
            variable->failures++;
            variable->Exclude(value);
            domain_size--;
            j--;
        }
    }
}

template <class T>
bool Problem<T>::EnforceArcConsistency(size_t v)
{
    bool domain_reduced;
    do {
        domain_reduced = false;

        Sort(v);
        for (size_t i = v; i < variables.size(); i++) {
            Variable<T> *variable = variables[i];
            if (variable->GetDomainSize() == 1)
                variable->active = false;
            if (variable->active == false)
                continue;

            size_t old_domain_size = variable->GetDomainSize();
            Revise(variable, v);
            size_t new_domain_size = variable->GetDomainSize();
            if (new_domain_size == 0)
                return false;
            if (new_domain_size == 1) {
                bool consistent = PropagateDecision(variable);
                assert(consistent);
            }
            if (new_domain_size < old_domain_size) {
                variable->ActivateAffectedVariables();
                domain_reduced = true;
            }
            variable->active = false;
            Sort(i+1);
        }
    } while (domain_reduced);
    return true;
}

template <class T>
void Problem<T>::Solve()
{
    if (option.optimize && GetSolutionCost() == LONG_MAX) {
        printf("Solution cost function required for optimization mode\n");
        return;
    }

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

    try {
        if (option.arc_consistency) {
            bool consistent = EnforceArcConsistency(0);
            if (!consistent)
                throw false;
        }

        storage.clear();

        for (size_t i = 0; i < constraints.size(); i++)
            constraints[i]->UpdateBounds();

        Sort(0);
        DEBUG( ShowState(NULL) );

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
        ProcessSolution();
        return;
    }

    Variable<T> *variable = variables[v];
    DEBUG( variable->ShowDomain() );

    bool deadend = true;
    while (variable->GetDomainSize() > 0) {
        StartCheckpoint();
        T value = variable->GetValue(0);

        variable->Decide(value);
        bool consistent = PropagateDecision(variable);
        if (consistent && option.arc_consistency) {
            variable->ActivateAffectedVariables();
            consistent = EnforceArcConsistency(v+1);
        }
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
        //variable->failures = deadend_count;
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
long Problem<T>::GetSolutionCost()
{
    return LONG_MAX;
}

template <class T>
bool Problem<T>::CheckSolution(size_t v)
{
    for (size_t i = v; i < variables.size(); i++)
        if (variables[i]->GetDomainSize() > 1)
            return false;
    ProcessSolution();
    return true;
}

template <class T>
void Problem<T>::ProcessSolution()
{
    num_solutions++;
    if (option.optimize) {
        long cost =  GetSolutionCost();
        if (min_cost > cost) {
            min_cost = cost;
            printf("----- Solution %ld after %ld searches, %ld deadends and %.3fs -----\n",
                    num_solutions, search_count, deadend_count, GetTimeUsage());
            ShowSolution();
        }
    } else {
        printf("----- Solution %ld after %ld searches, %ld deadends and %.3fs -----\n",
                num_solutions, search_count, deadend_count, GetTimeUsage());
        ShowSolution();
        if (option.num_solutions > 0 && num_solutions >= option.num_solutions) {
            throw true;
        }
    }
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
