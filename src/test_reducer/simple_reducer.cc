#include "../common/mapreduce.h"
#include "../common/reducer.h"
#include "../common/utils.h"

class SimpleReducer : public mapreduce::Reducer {
public:
    void reduce() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        while (get_next_pair(key, val)) {
            std::cerr << "[SIMPLE REDUCER IMPL] key: " << key
                      << ", val: " << val << std::endl;
            emit(key, val);
        }
    }
};

int main(int argc, char** argv)
{
    std::cerr << "[REDUCER WORKER] Starting binary" << std::endl;
    SimpleReducer reducer;

    reducer.start(argc, argv);

    return 0;
}