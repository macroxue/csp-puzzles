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

        struct Line {
            Set<M> value;
            Set<M> decided;

            bool operator ==(Line &line);
        };

        class Cache {
            public:
                bool Lookup(Line &in, Line &out);
                void Update(Line &in, Line &out);

            private:
                uint64_t Hash(Line &in);

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
        if (variables[i]->GetDomainSize() > 1 && out.decided.Has(i)) {
            variables[i]->Decide(out.value.Has(i));

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

    typedef typename Automaton<bool,2,M>::Input Input;
    Input input[num_inputs];
    Line in;

    for (size_t i = 0; i < num_inputs; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            input[i].value = variables[i]->GetValue(0);
            input[i].decided = true;
            if (input[i].value)
                in.value.Add(i);
            else
                in.value.Remove(i);
            in.decided.Add(i);
        } else {
            input[i].decided = false;
            in.value.Add(i);
        }
    }

    if (cache.Lookup(in, out))
        return out.decided.Has(M);

    if (!automaton.Accept(input, num_inputs)) {
        cache.Update(in, out);
        return false;
    }

    out.decided.Add(M);
    for (size_t i = 0; i < num_inputs; i++) {
        if (input[i].decided) {
            out.decided.Add(i);
            if (input[i].value)
                out.value.Add(i);
        }
    }
    cache.Update(in, out);

    return true;
}

template <size_t M>
bool RunLength<M>::Line::operator ==(Line &line)
{
    return value == line.value && decided == line.decided;
}

template <size_t M>
uint64_t RunLength<M>::Cache::Hash(Line &in)
{
    uint64_t h1 = in.value.Hash();
    uint64_t h2 = in.decided.Hash();
    uint64_t sum = 0;
    for (size_t i = 0; i < 64; i += 10)
        sum += (h1 >> i) + (h2 >> i);
    return sum % BUCKETS;
}

template <size_t M>
bool RunLength<M>::Cache::Lookup(Line &in, Line &out)
{
    uint64_t index = Hash(in);
    if (in == input[index]) {
        out = output[index];
        return true;
    } else
        return false;
}

template <size_t M>
void RunLength<M>::Cache::Update(Line &in, Line &out)
{
    uint64_t index = Hash(in);
    input[index]  = in;
    output[index] = out;
}

#endif

