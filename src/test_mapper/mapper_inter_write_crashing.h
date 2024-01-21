#ifndef MAPPER_RANDOMLY_CRASHING_H
#define MAPPER_RANDOMLY_CRASHING_H

#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/mapper.h"
#include "../common/test-utils.h"

/**
 * map() crashes before emitting in one in [odds] times after [steps] writes.
 * [steps] is randomly sampled from uniform distribution [1, steps_rng].
 * Before each emit calls wait_cpu(wait_n).
*/
class MapperInterWriteCrashing : public mapreduce::Mapper {
public:
    void map() override {
        mapreduce::key_t key;
        mapreduce::val_t val;

        bool fail = (rand_in_range(0, odds - 1) == 0);

        while (get_next_pair(key, val)) {
            std::cerr << "[MAPPER_INTER_WRITE_CRASHING IMPL] key: " << key << ", val: " << val << std::endl;
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
    MapperInterWriteCrashing(int odds, int steps, int wait_n) {
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
