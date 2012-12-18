#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <assert.h>

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
        struct State {
            size_t           dist_to_acc; // distance to accepting state
            State *          transition[N];

            State();
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
            vector<State *>        states;
            size_t                 num_inputs;
            T                      inputs[N];
            vector<Transition>     transition;

            void AddState(State *state);
            void AddInput(T value);
            void AddTransition(State *from, State *to, T value);
        };

        size_t              alphabet_size;
        T                   alphabet[N];
        State             * start;
        State             * accept;
        vector<State *>     all_states;
        vector<PowerState>  power_states;

        bool RejectState(size_t i, State *state, Input input[], size_t input_size);
};

template <class T, size_t N>
Automaton<T,N>::Automaton(vector<Run> &run)
{
    start = accept = new State;
    all_states.push_back(start);
    alphabet_size = 0;

    for (size_t i = 0; i < run.size(); i++) {
        // build alphabet
        T  value = run[i].value;
        size_t j;
        for (j = 0; j < alphabet_size; j++)
            if (alphabet[j] == value)
                break;
        if (j == alphabet_size)
            alphabet[alphabet_size++] = value;

        // build finite state machine
        for (size_t n = 0; n < run[i].count; n++) {
            State *new_state = new State;
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
}

template <class T, size_t N>
bool Automaton<T,N>::Accept(Input input[], size_t input_size)
{
    // initialize power states
    size_t new_size = input_size + 1;
    if (power_states.size() < new_size) 
        power_states.resize(new_size);

    for (size_t i = 0; i < new_size; i++) {
        power_states[i].states.clear();
        power_states[i].num_inputs = 0;
        power_states[i].transition.clear();
    }

    power_states[0].states.push_back(start);

    // construct power states and transitions based on input
    for (size_t i = 0; i < input_size; i++) {
        PowerState &power_state = power_states[i];
        vector<State *> &states = power_state.states;
        for (size_t s = 0; s < states.size(); s++) {
            bool valid = false;

            for (size_t v = 0; v < alphabet_size; v++) {
                T  value = alphabet[v];
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
                power_state.AddInput(value);
                power_states[i + 1].AddState(next_state);
                valid = true;
            }

            if (!valid) {
                valid = RejectState(i, states[s], input, input_size);
                if (!valid)
                    return false;
            }
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

    PowerState &power_state = power_states[i];
    power_state.num_inputs = 0;
    vector<Transition> &transition = power_state.transition;
    size_t size = transition.size();
    for (size_t t = 0; t < size; t++) {
        if (transition[t].to == state) {
            transition[t] = transition[--size];
            t--;
        } else {
            power_state.AddInput(transition[t].value);
        }
    }
    transition.erase(transition.begin() + size, transition.end());

    if (power_state.num_inputs == 0)
        return false;
    if (power_state.num_inputs == 1) {
        input[i].value   = power_state.inputs[0];
        input[i].decided = true;
    }

    size = power_state.states.size();
    for (size_t s = 0; s < size; s++) {
        State *from_state = power_state.states[s];
        bool valid = false;
        for (size_t t = 0; t < transition.size(); t++) {
            if (transition[t].from == from_state) {
                valid = true;
                break;
            }
        }
        if (!valid) {
            power_state.states[s] = power_state.states[--size];
            s--;
            RejectState(i, from_state, input, input_size);
        }
    }
    power_state.states.erase(power_state.states.begin() + size, power_state.states.end());

    return true;
}

template <class T, size_t N>
Automaton<T,N>::State::State()
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
    if (find(states.begin(), states.end(), state) == states.end())
        states.push_back(state);
}

template <class T, size_t N>
void Automaton<T,N>::PowerState::AddInput(T value) {
    for (size_t i = 0; i < num_inputs; i++)
        if (inputs[i] == value)
            return;
    inputs[num_inputs++] = value;
}

template <class T, size_t N>
void Automaton<T,N>::PowerState::AddTransition(State *from, State *to, T value)
{
    transition.push_back(Transition(from, to, value));
}

#endif
