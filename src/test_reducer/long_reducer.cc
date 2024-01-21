#include "../common/mapreduce.h"
#include "../common/reducer.h"
#include "../common/utils.h"

class LongReducer : public mapreduce::Reducer {
public:
    void reduce() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        sleep(2);
        while (get_next_pair(key, val)) {
            emit(key, val);
        }
    }
};

int main(int argc, char** argv)
{
    sleep(10);
    std::cerr << "[REDUCER WORKER] Starting binary" << std::endl;
    LongReducer reducer;

    reducer.start(argc, argv);

    return 0;
}