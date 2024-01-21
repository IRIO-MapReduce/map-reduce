#ifndef REDUCER_RANDOMLY_CRASHING_H
#define REDUCER_RANDOMLY_CRASHING_H

#include "../common/mapreduce.h"
#include "../common/reducer.h"
#include "../common/test-utils.h"
#include "../common/utils.h"

/**
 * reduce() crashes before emitting in one in [odds] times.
 */
class ReducerRandomlyCrashing : public mapreduce::Reducer {
public:
    void reduce() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        if (rand_in_range(0, odds - 1) == 0) {
            exit(1);
        }

        while (get_next_pair(key, val)) {
            std::cerr << "[REDUCER_RANDOMLY_CRASHING IMPL] key: " << key
                      << ", val: " << val << std::endl;
            emit(key, val);
            wait_cpu(wait_n);
        }
    }
    ReducerRandomlyCrashing(int odds, int wait_n)
    {
        this->odds = odds;
        this->wait_n = wait_n;
    }

private:
    int odds;
    int wait_n;
};

#endif
