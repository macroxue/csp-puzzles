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

        bool Accept(const Line &in, Line &out, size_t num_variables);
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
bool RunLength<M,B>::Enforce()
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    size_t num_variables = variables.size();

    Variable<bool> *  undecided_variables[num_variables];
    size_t            undecided_indexes[num_variables];
    size_t            num_undecided = 0;
    Line              in, out;

    for (size_t i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            in.SetValue(i, variables[i]->GetValue(0));
            in.SetDecided(i);
        } else {
            undecided_variables[num_undecided] = variables[i];
            undecided_indexes[num_undecided] = i;
            num_undecided++;
        }
    }

    if (!Accept(in, out, num_variables))
        return false;

    for (size_t i = 0; i < num_undecided; i++) {
        if (out.IsDecided(undecided_indexes[i])) {
            undecided_variables[i]->Decide(out.GetValue(undecided_indexes[i]));
            undecided_variables[i]->PropagateDecision(this);
        }
    }

    return true;
}

template <size_t M, size_t B>
bool RunLength<M,B>::Accept(const Line &in, Line &out, size_t num_variables)
{
    problem->IncrementCounter(0);
    if (cache.Lookup(in, out)) {
        problem->IncrementCounter(1);
        return out.IsDecided(M); // accept flag is the last bit
    }

    if (automaton.Accept(in, out, num_variables)) {
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
    // Only need to take care of the default Line value
    uint64_t index = Line().Hash();
    input[index].SetValue(0, true);
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

