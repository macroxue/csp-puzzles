#ifndef VARIABLE_H
#define VARIABLE_H

#include "Domain.h"

#include <stdio.h>
#include <vector>
using namespace std;

template <class T>
class Constraint;

template <class T>
class Variable
{
    public:
        Variable(T low, T high);
        Variable(const T value[]);
        Variable(const T value[], size_t count);
        Variable(bool (*generator_fn)(void *, T &), void *cookie);
        void Init();

        void SetName(const char *name);
        const char *GetName() const;
        void SetId(size_t id);
        size_t GetId() const;

        void AddConstraint(Constraint<T> *constraint);
        bool PropagateDecision(Constraint<T> *start);
        bool PropagateReduction(Constraint<T> *start);
        void OnUpdate();
        void Decide(T value);
        bool Exclude(T value);
        void ExcludeAt(size_t i);

        T    GetValue(size_t i) const;
        size_t GetDomainSize() const;
        const Domain<T> & GetDomain() const;
        void  ShowDomain() const;
        vector<Constraint<T> *> & GetConstraints();

        typedef vector< pair<Variable<T> *, size_t> > Storage;
        void SetStorage(Storage *storage);
        void Checkpoint();
        void Restore(size_t count);

    private:
        Domain<T>                domain;
        vector<Constraint<T> *>  constraints;
        Storage *                storage;
        const char *             name;
        size_t                      id;

    public:
        size_t                      failures;
};

#include "Constraint.h"
#include "Problem.h"

template <class T>
Variable<T>::Variable(T low, T high)
    : domain(low, high), name("")
{
    Init();
}

template <class T>
Variable<T>::Variable(const T value[])
    : domain(value), name("")
{
    Init();
}

template <class T>
Variable<T>::Variable(const T value[], size_t count)
    : domain(value, count), name("")
{
    Init();
}

template <class T>
Variable<T>::Variable(bool (*generator_fn)(void *, T &), void *cookie)
    : domain(generator_fn, cookie), name("")
{
    Init();
}

template <class T>
void Variable<T>::Init()
{
    name       = "";
    failures   = 0;
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
void Variable<T>::SetId(size_t id_in)
{
    id = id_in;
}

template <class T>
size_t  Variable<T>::GetId() const
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
    for (size_t i = 0; i < constraints.size(); i++) {
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
    for (size_t i = 0; i < constraints.size(); i++) {
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
    size_t pos = domain.Find(value);
    if (pos != domain.GetSize()) {
        ExcludeAt(pos);
        return true;
    } else
        return false;
}

template <class T>
void Variable<T>::ExcludeAt(size_t i)
{
    OnUpdate();
    domain.EraseValueAt(i);
}

template <class T>
T    Variable<T>::GetValue(size_t i) const
{
    return domain[i];
}

template <class T>
size_t  Variable<T>::GetDomainSize() const
{
    return domain.GetSize();
}

template <class T>
const Domain<T> & Variable<T>::GetDomain() const
{
    return domain;
}

template <class T>
void Variable<T>::ShowDomain() const
{
    printf("Variable %ld = { ", GetId());
    for (size_t i = 0; i < GetDomainSize(); i++)
        printf("%d ", GetValue(i));
    printf("}\n");
}

template <class T>
vector<Constraint<T> *> & Variable<T>::GetConstraints()
{
    return constraints;
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
void Variable<T>::Restore(size_t count)
{
    domain.SetCount(count);
}

#endif
