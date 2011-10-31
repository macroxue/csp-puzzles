#ifndef RUNLENGTH_H
#define RUNLENGTH_H

#include "Constraint.h"

//
// RunLength: 
//
class RunLength : public Constraint<bool>
{
    public:
        RunLength(vector<int> & length);
        bool OnDecided(Variable<bool> *decided);

    private:
        bool Match(int num_values, int values[]);
        vector<int> & length;
        int  min_span;
};

RunLength::RunLength(vector<int> & length)
    : length(length)
{
    min_span = -1;
    for (unsigned i = 0; i < length.size(); i++)
        min_span += 1 + length[i];
}

bool RunLength::OnDecided(Variable<bool> *decided)
{
    vector<Variable<bool> *>  &variables = Constraint<bool>::variables;
    int num_variables = variables.size();

    int cur_run = -1, num_runs = length.size();
    int run_length = 0, required_span = min_span;
    for (int i = 0; i < num_variables; i++) {
        if (variables[i]->GetDomainSize() == 1) {
            if (variables[i]->GetValue(0) == true) {
                if (run_length == 0) {
                    cur_run++;
                    if (cur_run >= num_runs)
                        return false;
                }
                run_length++;
                if (run_length > length[cur_run])
                    return false;
            } else {
                if (run_length > 0) {
                    if (run_length != length[cur_run])
                        return false;
                    required_span -= 1 + run_length;
                    run_length = 0;
                }
            }
        } else {
            if (required_span > (num_variables - i) + run_length)
                return false;
            return true;
        }
    }
    if (cur_run != num_runs-1)
        return false;
    if (run_length > 0 && run_length != length[cur_run])
        return false;

    return true;
}

#endif

