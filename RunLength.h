#ifndef RUNLENGTH_H
#define RUNLENGTH_H

#include "Constraint.h"
#include "Automaton.h"

//
// RunLength: 
//
template <size_t M>
class RunLength : public Constraint<bool>
{
    public:
        RunLength(vector<int> & length);
        bool OnDecided(Variable<bool> *decided);
        bool Enforce();

    private:
        vector<int> & length;
        Automaton<bool,2,M>  automaton;

        class Line : public Automaton<bool,2,M>::template Input<Line> {
            public:
                bool operator ==(Line &line);
                void Show() const;

                bool IsDecided(size_t i) const  { return decided.Has(i); }
                void SetDecided(size_t i)       { decided.Add(i); }
                bool GetValue(size_t i) const   { return value.Has(i); }
                void SetValue(size_t i, bool v) { if (v) value.Add(i); else value.Remove(i); }

            private:
                Set<M> value;
                Set<M> decided;
        };

        class Cache {
            public:
                bool Lookup(Line &in, Line &out);
                void Update(Line &in, Line &out);

                uint64_t Hash(Line &in) const;
                size_t   line_length;
            private:
                static const size_t BUCKETS = 1024;
                Line input[BUCKETS];
                Line output[BUCKETS];
        };

        Cache cache;

        bool Accept(Line &out);
};

template <size_t M>
RunLength<M>::RunLength(vector<int> & length)
    : length(length)
{
    typedef typename Automaton<bool,2,M>::Run   Run;
    vector<Run> run;

    for (size_t i = 0; i < length.size(); i++) {
        run.push_back(Run(false, i != 0, Run::AT_LEAST));
        run.push_back(Run(true, length[i], Run::EQUAL));
    }
    run.push_back(Run(false, 0, Run::AT_LEAST));

    new (&automaton) Automaton<bool,2,M>(run);
}

template <size_t M>
bool RunLength<M>::OnDecided(Variable<bool> *decided)
{
    return Enforce();
}

template <size_t M>
bool RunLength<M>::Enforce()
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_variables = variables.size();

    Line out;
    if (!Accept(out))
        return false;

    for (size_t i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() > 1 && out.IsDecided(i)) {
            variables[i]->Decide(out.GetValue(i));

            vector<Constraint<bool>*> &constraints = variables[i]->GetConstraints();
            for (size_t j = 0; j < constraints.size(); j++) {
                if (constraints[j] != this)
                    problem->ActivateConstraint(constraints[j]);
            }
        }
    }

    return true;
}

template <size_t M>
bool RunLength<M>::Accept(Line &out)
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_inputs = variables.size();

    Line in;
    for (size_t i = 0; i < num_inputs; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            in.SetValue(i, variables[i]->GetValue(0));
            in.SetDecided(i);
        } else {
            in.SetValue(i, true);
        }
    }

    cache.line_length = num_inputs;
    if (cache.Lookup(in, out))
        return out.IsDecided(M); // accept flag is the last bit

    out = in;
    if (automaton.Accept(out, num_inputs)) {
        out.SetDecided(M); // accept flag is the last bit
        cache.Update(in, out);
        // printf("%p %zu accepts:", this, in.Hash() % 1024); in.Show(); putchar('\n');
        return true;
    } else {
        cache.Update(in, out);
        // printf("%p %zu rejects:", this, in.Hash() % 1024); in.Show(); putchar('\n');
        return false;
    }
}

template <size_t M>
bool RunLength<M>::Line::operator ==(Line &line)
{
    return value == line.value && decided == line.decided;
}
                
template <size_t M>
void RunLength<M>::Line::Show() const
{
    for (size_t i = 0; i < M; i++)
        putchar(IsDecided(i) ? GetValue(i) + '0' : '?');
}

template <size_t M>
uint64_t RunLength<M>::Cache::Hash(Line &in) const
{ 
    extern uint64_t hash_rand[4][128];

    uint64_t sum = 0;
    for (size_t i = 0; i < line_length; i++)
        sum += hash_rand[in.GetValue(i) + in.IsDecided(i)*2][i];
    return sum;
}

template <size_t M>
bool RunLength<M>::Cache::Lookup(Line &in, Line &out)
{
    uint64_t index = Hash(in) % BUCKETS;
    if (in == input[index]) {
        out = output[index];
        return true;
    } else
        return false;
}

template <size_t M>
void RunLength<M>::Cache::Update(Line &in, Line &out)
{
    uint64_t index = Hash(in) % BUCKETS;
    input[index]  = in;
    output[index] = out;
}

#endif

