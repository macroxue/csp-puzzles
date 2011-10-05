#ifndef VARIABLE_H
#define VARIABLE_H

#include "Domain.h"

#include <vector>
using namespace std;

template <class T>
class Constraint;

template <class T>
class Variable
{
    public:
        Variable(T low, T high);
        Variable(T value[], int count);
        Variable(bool (*generator_fn)(void *, T &), void *cookie);

        void SetName(const char *name);
        const char *GetName() const;
        void SetId(int id);
        int  GetId() const;

        void AddConstraint(Constraint<T> *constraint);
        bool PropagateDecision(Constraint<T> *start);
        bool PropagateReduction(Constraint<T> *start);
        void OnUpdate();
        void Decide(T value);
        bool Exclude(T value);

        T    GetValue(int i) const;
        int  GetDomainSize() const;
        const Domain<T> & GetDomain() const;

        typedef vector< pair<Variable<T> *, int> > Storage;
        void SetStorage(Storage *storage);
        void Checkpoint();
        void Restore(int count);

    private:
        Domain<T>                domain;
        vector<Constraint<T> *>  constraints;
        Storage *                storage;
        const char *             name;
        int                      id;
};

#include "Constraint.h"

template <class T>
Variable<T>::Variable(T low, T high)
    : domain(low, high), name("")
{
}

template <class T>
Variable<T>::Variable(T value[], int count)
    : domain(value, count), name("")
{
}

template <class T>
Variable<T>::Variable(bool (*generator_fn)(void *, T &), void *cookie)
    : domain(generator_fn, cookie), name("")
{
}

template <class T>
void Variable<T>::SetName(const char *name_in)
{
    name = name_in;
}

template <class T>
const char *Variable<T>::GetName() const
{
    return name;
}

template <class T>
void Variable<T>::SetId(int id_in)
{
    id = id_in;
}

template <class T>
int  Variable<T>::GetId() const
{
    return id;
}

template <class T>
void Variable<T>::AddConstraint(Constraint<T> *constraint)
{
    constraints.push_back(constraint);
}

template <class T>
bool Variable<T>::PropagateDecision(Constraint<T> *start)
{
    for (unsigned i = 0; i < constraints.size(); i++) {
        if (constraints[i] == start) continue;

        bool consistent = constraints[i]->OnDecided(this);
        if (!consistent)
            return false;
    }
    return true;
}

template <class T>
bool Variable<T>::PropagateReduction(Constraint<T> *start)
{
    for (unsigned i = 0; i < constraints.size(); i++) {
        if (constraints[i] == start)
            continue;

        bool consistent = constraints[i]->OnReduced(this);
        if (!consistent)
            return false;
    }
    return true;
}

template <class T>
void Variable<T>::OnUpdate()
{
    Checkpoint();
}

template <class T>
void Variable<T>::Decide(T value)
{
    // Assert: value must be in the domain.
    OnUpdate();
    domain.LimitBounds(value, value);
}

template <class T>
bool Variable<T>::Exclude(T value)
{
    int pos = domain.Find(value);
    if (pos != -1) {
        OnUpdate();
        domain.EraseValueAt(pos);
        return true;
    } else
        return false;
}

template <class T>
T    Variable<T>::GetValue(int i) const
{
    return domain[i];
}

template <class T>
int  Variable<T>::GetDomainSize() const
{
    return domain.GetSize();
}

template <class T>
const Domain<T> & Variable<T>::GetDomain() const
{
    return domain;
}

template <class T>
void Variable<T>::SetStorage(Storage *storage_in)
{
    storage = storage_in;
}

template <class T>
void Variable<T>::Checkpoint()
{
    storage->push_back(make_pair(this, domain.GetCount()));
}

template <class T>
void Variable<T>::Restore(int count)
{
    domain.SetCount(count);
}

#endif
