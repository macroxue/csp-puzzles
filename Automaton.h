#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <assert.h>

#include <algorithm>
#include <map>
#include <vector>
using namespace std;

template <class T>
class Automaton
{
    public:

        struct Run {
            enum Mod { EQUAL, AT_LEAST };

            T    value;
            int  count;
            Mod  mod;

            Run(T value, int count, Mod mod)
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
        bool Accept(Input input[], int input_size);

    private:
        struct State {
            int              dist_to_acc; // distance to accepting state
            map<T, State *>  transition;

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
            vector<T>              inputs;
            vector<Transition>     transition;

            void AddState(State *state);
            void AddInput(T value);
            void AddTransition(State *from, State *to, T value);
        };

        vector<T>           alphabet;
        State             * start;
        State             * accept;
        vector<State *>     all_states;
        vector<PowerState>  power_states;

        bool RejectState(int i, State *state, Input input[], int input_size);
};

template <class T>
Automaton<T>::Automaton(vector<Run> &run)
{
    start = accept = new State;
    all_states.push_back(start);

    for (unsigned i = 0; i < run.size(); i++) {
        // build alphabet
        T  value = run[i].value;
        if (find(alphabet.begin(), alphabet.end(), value) == alphabet.end())
            alphabet.push_back(value);

        // build finite state machine
        for (int n = 0; n < run[i].count; n++) {
            State *new_state = new State;
            all_states.push_back(new_state);
            accept->transition[value] = new_state;
            accept = new_state;
        }
        if (run[i].mod == Run::AT_LEAST)
            accept->transition[value] = accept;
    }

    // caculate distance to accepting state
    for (unsigned i = 0; i < all_states.size(); i++)
        all_states[i]->dist_to_acc = all_states.size() - 1 - i;
    assert(accept->dist_to_acc == 0);
}

template <class T>
bool Automaton<T>::Accept(Input input[], int input_size)
{
    // initialize power states
    size_t new_size = input_size + 1;
    if (power_states.size() < new_size) 
        power_states.resize(new_size);

    for (unsigned i = 0; i < new_size; i++) {
        power_states[i].states.clear();
        power_states[i].inputs.clear();
        power_states[i].transition.clear();
    }

    power_states[0].states.push_back(start);

    // construct power states and transitions based on input
    for (int i = 0; i < input_size; i++) {
        PowerState &power_state = power_states[i];
        vector<State *> &states = power_state.states;
        for (unsigned s = 0; s < states.size(); s++) {
            bool valid = false;

            for (unsigned v = 0; v < alphabet.size(); v++) {
                T  value = alphabet[v];
                if (input[i].decided && value != input[i].value)
                    continue;

                State *next_state = states[s]->Transit(value);
                if (next_state == NULL)
                    continue;

                // reject state that's too far from accepting
                int num_inputs_left = input_size - 1 - i;
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

        if (power_state.inputs.size() == 0)
            return false;
        if (power_state.inputs.size() == 1) {
            input[i].value   = power_state.inputs[0];
            input[i].decided = true;
        }
    }

    return true;
}

template <class T>
bool Automaton<T>::RejectState(int i, State *state, Input input[], int input_size)
{
    if (i == 0)
        return true;
    i--;

    PowerState &power_state = power_states[i];
    power_state.inputs.clear();
    vector<Transition> &transition = power_state.transition;
    for (unsigned t = 0; t < transition.size(); t++) {
        if (transition[t].to == state) {
            transition.erase(transition.begin() + t);
            t--;
        } else {
            power_state.AddInput(transition[t].value);
        }
    }

    if (power_state.inputs.size() == 0)
        return false;
    if (power_state.inputs.size() == 1) {
        input[i].value   = power_state.inputs[0];
        input[i].decided = true;
    }

    for (unsigned s = 0; s < power_state.states.size(); s++) {
        State *from_state = power_state.states[s];
        bool valid = false;
        for (unsigned t = 0; t < transition.size(); t++) {
            if (transition[t].from == from_state)
                valid = true;
        }
        if (!valid) {
            power_state.states.erase(power_state.states.begin() + s);
            s--;
            RejectState(i, from_state, input, input_size);
        }
    }

    return true;
}

template <class T>
typename Automaton<T>::State *Automaton<T>::State::Transit(T value)
{
    typename map<T, State *>::iterator it = transition.find(value);
    if (it == transition.end())
        return NULL;
    else
        return it->second;
}

template <class T>
void Automaton<T>::PowerState::AddState(State *state) 
{
    if (find(states.begin(), states.end(), state) == states.end())
        states.push_back(state);
}

template <class T>
void Automaton<T>::PowerState::AddInput(T value) {
    if (find(inputs.begin(), inputs.end(), value) == inputs.end())
        inputs.push_back(value);
}

template <class T>
void Automaton<T>::PowerState::AddTransition(State *from, State *to, T value)
{
    transition.push_back(Transition(from, to, value));
}

#endif
