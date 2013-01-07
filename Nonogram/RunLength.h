#ifndef RUNLENGTH_H
#define RUNLENGTH_H

#include "Constraint.h"
#include "Automaton.h"

//
// RunLength: 
//
template <size_t M, size_t B>
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
                bool operator ==(const Line &line) const;
                void Show() const;

                bool IsDecided(size_t i) const  { return decided.Has(i); }
                void SetDecided(size_t i)       { decided.Add(i); }
                bool GetValue(size_t i) const   { return value.Has(i); }
                void SetValue(size_t i, bool v) { if (v) value.Add(i); else value.Remove(i); }

                uint64_t Hash() const;

            private:
                Set<M+1> value;
                Set<M+1> decided;
        };

        class Cache {
            public:
                Cache();
                bool Lookup(const Line &in, Line &out) const;
                void Update(const Line &in, const Line &out);

            private:
                Line input[1<<B];
                Line output[1<<B];
        };

        Cache cache;

        bool Accept(Line &out);
};

template <size_t M, size_t B>
RunLength<M,B>::RunLength(vector<int> & length)
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

template <size_t M, size_t B>
bool RunLength<M,B>::OnDecided(Variable<bool> *decided)
{
    return Enforce();
}

template <size_t M, size_t B>
bool RunLength<M,B>::Enforce()
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

template <size_t M, size_t B>
bool RunLength<M,B>::Accept(Line &out)
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_inputs = variables.size();

    Line in;
    for (size_t i = 0; i < num_inputs; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            in.SetValue(i, variables[i]->GetValue(0));
            in.SetDecided(i);
        }
    }

    GetProblem()->IncrementCounter(0);
    if (cache.Lookup(in, out)) {
        GetProblem()->IncrementCounter(1);
        return out.IsDecided(M); // accept flag is the last bit
    }

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

template <size_t M, size_t B>
bool RunLength<M,B>::Line::operator ==(const Line &line) const
{
    return value == line.value && decided == line.decided;
}
                
template <size_t M, size_t B>
void RunLength<M,B>::Line::Show() const
{
    for (size_t i = 0; i < M; i++)
        putchar(IsDecided(i) ? GetValue(i) + '0' : '?');
}

template <size_t M, size_t B>
uint64_t RunLength<M,B>::Line::Hash() const
{
    extern __uint128_t hash_rand[2][2];
    return (value.Hash(hash_rand[0], B) + decided.Hash(hash_rand[1], B)) % (1 << B);
}

template <size_t M, size_t B>
RunLength<M,B>::Cache::Cache()
{
    // Initialze cache to have invalid inputs
    for (size_t i = 0; i < B; i++)
        input[i].SetValue(M, true);
}

template <size_t M, size_t B>
bool RunLength<M,B>::Cache::Lookup(const Line &in, Line &out) const
{
    uint64_t index = in.Hash();
    if (in == input[index]) {
        out = output[index];
        return true;
    } else
        return false;
}

template <size_t M, size_t B>
void RunLength<M,B>::Cache::Update(const Line &in, const Line &out)
{
    uint64_t index = in.Hash();
    input[index]  = in;
    output[index] = out;
}

#endif

