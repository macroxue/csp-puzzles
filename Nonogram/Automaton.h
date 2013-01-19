#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <assert.h>

#include <vector>
using namespace std;

#include "Set.h"

template <class T, size_t N, size_t M>
class Automaton
{
    public:

        struct Run {
            enum Mod { EQUAL, AT_LEAST };

            T      value;
            size_t count;
            Mod    mod;

            Run(T value, size_t count, Mod mod)
                : value(value), count(count), mod(mod) {}
        };

        template <class I>
            struct Input {
                bool IsDecided(size_t i) const { return static_cast<const I *>(this)->IsDecided(i); }
                void SetDecided(size_t i)      { static_cast<I *>(this)->SetDecided(i); }
                T    GetValue(size_t i) const  { return static_cast<const I *>(this)->GetValue(i); }
                void SetValue(size_t i, T v)   { static_cast<I *>(this)->SetValue(i, v); }
            };

        Automaton() {}
        Automaton(vector<Run> &run);
        template <class I>
            bool Accept(const Input<I> &input, Input<I> &output, size_t input_size);

    private:

        struct State;

        struct Transition {
            State *           from;
            State *           to;
            T                 value;
        };

        struct State {
            size_t            id;
            size_t            dist_to_acc; // distance to accepting state
            Transition        transitions[N];

            State(size_t id = 0);
        }; 

        struct PowerState {
            Set<M>            member_states;
            size_t            num_states;
            State *           states[M];

            void AddState(State *state);
        };

        struct PowerInput {
            Set<N>            member_inputs;
            size_t            num_inputs;
            T                 inputs[N];

            void AddInput(T value);
        };

        State *               start;
        State *               accept;
        vector<State *>       all_states;
        vector<Transition *>  power_transition[M];
};

template <class T, size_t N, size_t M>
Automaton<T,N,M>::Automaton(vector<Run> &run)
{
    start = accept = new State(0);
    all_states.push_back(start);

    for (size_t i = 0; i < run.size(); i++) {
        // Build finite state machine
        T  value = run[i].value;
        for (size_t n = 0; n < run[i].count; n++) {
            State *new_state = new State(all_states.size());
            all_states.push_back(new_state);
            accept->transitions[value].to = new_state;
            accept = new_state;
        }
        if (run[i].mod == Run::AT_LEAST)
            accept->transitions[value].to = accept;
    }

    // Calculate distance to accepting state
    for (size_t i = 0; i < all_states.size(); i++)
        all_states[i]->dist_to_acc = all_states.size() - 1 - i;
    assert(accept->dist_to_acc == 0);
    assert(all_states.size() <= M);
}

template <class T, size_t N, size_t M> template <class I>
bool Automaton<T,N,M>::Accept(const Input<I> &input, Input<I> &output, size_t input_size)
{
    // Initialize power states
    PowerState power_states[2];
    PowerState *power_state = &power_states[0], *next_power_state = &power_states[1];
    next_power_state->num_states = 0;
    next_power_state->AddState(start);

    // Construct power states and transitions based on input
    for (size_t i = 0; i < input_size; i++) {

        swap(power_state, next_power_state);
        next_power_state->member_states.Clear();
        next_power_state->num_states = 0;
        power_transition[i].clear();

        for (size_t s = 0; s < power_state->num_states; s++) {
            State *state = power_state->states[s];
            for (size_t v = 0; v < N; v++) {
                Transition &transition = state->transitions[v];

                // Skip invalid transition
                if (transition.to == NULL)
                    continue;

                // Skip mismatched input
                if (input.IsDecided(i) && transition.value != input.GetValue(i))
                    continue;

                // Reject state that's too far from accepting
                size_t num_inputs_left = input_size - 1 - i;
                if (transition.to->dist_to_acc > num_inputs_left) 
                    continue;

                power_transition[i].push_back(&transition);
                next_power_state->AddState(transition.to);
            }
        }

        if (next_power_state->num_states == 0)
            return false;
    }

    // Prune deadends backwards
    for (size_t i = input_size; i > 0; i--) {

        // Gather valid states and inputs
        PowerInput  power_input;
        power_input.num_inputs = 0;
        power_state->member_states.Clear();
        for (size_t t = 0; t < power_transition[i-1].size(); t++) {
            Transition &transition = *power_transition[i-1][t];
            if (next_power_state->member_states.Has(transition.to->id)) {
                power_state->member_states.Add(transition.from->id);

                // Only care about whether there is a single input
                if (power_input.num_inputs <= 1)
                    power_input.AddInput(transition.value);
            }
        }

        if (power_input.num_inputs == 1) {
            output.SetValue(i-1, power_input.inputs[0]);
            output.SetDecided(i-1);
        }

        swap(power_state, next_power_state);
    }

    return true;
}

template <class T, size_t N, size_t M>
Automaton<T,N,M>::State::State(size_t id)
    : id(id)
{
    for (size_t v = 0; v < N; v++) {
        transitions[v].from  = this;
        transitions[v].to    = NULL;
        transitions[v].value = (T)v;
    }
}

template <class T, size_t N, size_t M>
void Automaton<T,N,M>::PowerState::AddState(State *state) 
{
    if (!member_states.Has(state->id)) {
        member_states.Add(state->id);
        states[num_states++] = state;
    }
}

template <class T, size_t N, size_t M>
void Automaton<T,N,M>::PowerInput::AddInput(T value) 
{
    if (!member_inputs.Has(value)) {
        member_inputs.Add(value);
        inputs[num_inputs++] = value;
    }
}

#endif
