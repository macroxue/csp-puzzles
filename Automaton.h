#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <vector>
using namespace std;

template <class T, size_t N>
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

        struct Input {
            T    value;
            bool decided;

            Input() {}
            Input(T value, bool decided)
                : value(value), decided(decided) {}
        };

        Automaton() {}
        Automaton(vector<Run> &run);
        bool Accept(Input input[], size_t input_size);

    private:

        class Set {
            private:
                uint64_t  bits[2];
                uint64_t  Mask(size_t bit)   { return (uint64_t)1 << bit; }
            public:
                Set()                        { Clear(); }
                void      Clear()            { bits[0] = bits[1] = 0; }
                void      Add(size_t bit)    { bits[bit >> 6] |= Mask(bit & 63); }
                void      Remove(size_t bit) { bits[bit >> 6] &= ~Mask(bit & 63); }
                uint64_t  Has(size_t bit)    { return bits[bit >> 6] & Mask(bit & 63); }
        };

        static const size_t M = sizeof(Set) * 8;

        struct State {
            size_t           id;
            size_t           dist_to_acc; // distance to accepting state
            State *          transition[N];

            State(size_t id = 0);
            State *Transit(T value);
        }; 

        struct Transition {
            State   *from;
            State   *to;
            T       value;

            Transition(State *from, State *to, T value)
                : from(from), to(to), value(value) {}
        };

        struct PowerState {
            Set                    member_states;
            size_t                 num_states;
            State *                states[M];
            Set                    member_inputs;
            size_t                 num_inputs;
            T                      inputs[N];
            vector<Transition>     transition;

            void AddState(State *state);
            void RemoveState(State *state, size_t s);
            void AddInput(T value);
            void RemoveInput(T value, size_t j);
            void AddTransition(State *from, State *to, T value);
        };

        State             * start;
        State             * accept;
        vector<State *>     all_states;
        vector<PowerState>  power_states;

        bool RejectState(size_t i, State *state, Input input[], size_t input_size);
};

template <class T, size_t N>
Automaton<T,N>::Automaton(vector<Run> &run)
{
    start = accept = new State(0);
    all_states.push_back(start);

    for (size_t i = 0; i < run.size(); i++) {
        // build finite state machine
        T  value = run[i].value;
        for (size_t n = 0; n < run[i].count; n++) {
            State *new_state = new State(all_states.size());
            all_states.push_back(new_state);
            accept->transition[value] = new_state;
            accept = new_state;
        }
        if (run[i].mod == Run::AT_LEAST)
            accept->transition[value] = accept;
    }

    // caculate distance to accepting state
    for (size_t i = 0; i < all_states.size(); i++)
        all_states[i]->dist_to_acc = all_states.size() - 1 - i;
    assert(accept->dist_to_acc == 0);
    assert(all_states.size() <= M);
}

template <class T, size_t N>
bool Automaton<T,N>::Accept(Input input[], size_t input_size)
{
    // initialize power states
    size_t new_size = input_size + 1;
    if (power_states.size() < new_size) 
        power_states.resize(new_size);

    for (size_t i = 0; i < new_size; i++) {
        power_states[i].member_states.Clear();
        power_states[i].num_states = 0;
        power_states[i].member_inputs.Clear();
        power_states[i].num_inputs = 0;
        power_states[i].transition.clear();
    }

    power_states[0].AddState(start);

    // construct power states and transitions based on input
    for (size_t i = 0; i < input_size; i++) {
        PowerState &power_state = power_states[i];
        State **states = power_state.states;
        for (size_t s = 0; s < power_state.num_states; s++) {
            bool valid = false;

            for (size_t v = 0; v < N; v++) {
                T  value = (T)v;
                if (input[i].decided && value != input[i].value)
                    continue;

                State *next_state = states[s]->Transit(value);
                if (next_state == NULL)
                    continue;

                // reject state that's too far from accepting
                size_t num_inputs_left = input_size - 1 - i;
                if (next_state->dist_to_acc > num_inputs_left) 
                    continue;

                power_state.AddTransition(states[s], next_state, value);
                power_state.member_inputs.Add(value);
                power_states[i + 1].AddState(next_state);
                valid = true;
            }

            if (!valid) {
                valid = RejectState(i, states[s], input, input_size);
                if (!valid)
                    return false;
            }
        }

        // build inputs from member set
        power_state.num_inputs = 0;
        for (size_t v = 0; v < N; v++) {
            T  value = (T)v;
            if (power_state.member_inputs.Has(value))
                power_state.inputs[power_state.num_inputs++] = value;
        }

        if (power_state.num_inputs == 0)
            return false;
        if (power_state.num_inputs == 1) {
            input[i].value   = power_state.inputs[0];
            input[i].decided = true;
        }
    }

    return true;
}

template <class T, size_t N>
bool Automaton<T,N>::RejectState(size_t i, State *state, Input input[], size_t input_size)
{
    if (i == 0)
        return true;
    i--;

    // compute the sets of from-states and inputs, after removal of to-states
    Set member_states, member_inputs;
    PowerState &power_state = power_states[i];
    vector<Transition> &transition = power_state.transition;
    size_t size = transition.size();
    for (size_t t = 0; t < size; t++) {
        if (transition[t].to == state) {
            transition[t] = transition[--size];
            t--;
        } else {
            member_states.Add(transition[t].from->id);
            member_inputs.Add(transition[t].value);
        }
    }
    transition.erase(transition.begin() + size, transition.end());

    // remove unacceptable inputs
    for (size_t j = 0; j < power_state.num_inputs; j++) {
        T value = power_state.inputs[j];
        if (!member_inputs.Has(value)) {
            power_state.RemoveInput(value, j);
            j--;
        }
    }

    if (power_state.num_inputs == 0)
        return false;
    if (power_state.num_inputs == 1) {
        input[i].value   = power_state.inputs[0];
        input[i].decided = true;
    }

    // remove unreachable from-states
    for (size_t s = 0; s < power_state.num_states; s++) {
        State *from_state = power_state.states[s];
        if (!member_states.Has(from_state->id)) {
            power_state.RemoveState(from_state, s);
            s--;
            RejectState(i, from_state, input, input_size);
        }
    }

    return true;
}

template <class T, size_t N>
Automaton<T,N>::State::State(size_t id)
    : id(id)
{
    for (size_t i = 0; i < N; i++)
        transition[i] = NULL;
}

template <class T, size_t N>
typename Automaton<T,N>::State *Automaton<T,N>::State::Transit(T value)
{
    return transition[value];
}

template <class T, size_t N>
void Automaton<T,N>::PowerState::AddState(State *state) 
{
    if (!member_states.Has(state->id)) {
        member_states.Add(state->id);
        states[num_states++] = state;
    }
}

template <class T, size_t N>
void Automaton<T,N>::PowerState::RemoveState(State *state, size_t s)
{
    member_states.Remove(state->id);
    states[s] = states[--num_states];
}

template <class T, size_t N>
void Automaton<T,N>::PowerState::AddInput(T value) {
    if (!member_inputs.Has(value)) {
        member_inputs.Add(value);
        inputs[num_inputs++] = value;
    }
}

template <class T, size_t N>
void Automaton<T,N>::PowerState::RemoveInput(T value, size_t j)
{
    member_inputs.Remove(value);
    inputs[j] = inputs[--num_inputs];
}

template <class T, size_t N>
void Automaton<T,N>::PowerState::AddTransition(State *from, State *to, T value)
{
    transition.push_back(Transition(from, to, value));
}

#endif
