#ifndef REDUCER_INTER_WRITE_CRASHING_H
#define REDUCER_INTER_WRITE_CRASHING_H

#include "../common/mapreduce.h"
#include "../common/reducer.h"
#include "../common/test-utils.h"
#include "../common/utils.h"

/**
 * reduce() crashes before emitting in one in [odds] times after [steps] writes.
 * [steps] is randomly sampled from uniform distribution [1, steps_rng]
 */
class ReducerInterWriteCrashing : public mapreduce::Reducer {
public:
    void reduce() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        bool fail = (rand_in_range(0, odds - 1) == 0);

        while (get_next_pair(key, val)) {
            emit(key, val);
            wait_cpu(wait_n);
            if (fail) {
                steps--;
                if (steps == 0) {
                    exit(1);
                }
            }
        }
    }
    ReducerInterWriteCrashing(int odds, int steps, int wait_n)
    {
        this->odds = odds;
        this->steps = steps;
        this->wait_n = wait_n;
    }

private:
    int odds;
    int steps;
    int wait_n;
};

#endif
