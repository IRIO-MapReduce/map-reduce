#ifndef MAPPER_RANDOMLY_CRASHING_H
#define MAPPER_RANDOMLY_CRASHING_H

#include "../common/mapper.h"
#include "../common/mapreduce.h"
#include "../common/test-utils.h"
#include "../common/utils.h"

/**
 * map() crashes before emitting in one in [odds] times.
 */
class MapperRandomlyCrashing : public mapreduce::Mapper {
public:
    void map() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        bool fail = (rand_in_range(0, odds - 1) == 0);
        if (fail) {
            exit(1);
        }

        while (get_next_pair(key, val)) {
            emit(key, val);
            wait_cpu(wait_n);
        }
    }
    MapperRandomlyCrashing(int odds, int wait_n)
    {
        this->odds = odds;
        this->wait_n = wait_n;
    }

private:
    int odds;
    int wait_n;
};

#endif
