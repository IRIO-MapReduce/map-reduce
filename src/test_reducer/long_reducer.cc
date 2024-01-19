#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/reducer.h"

class LongReducer : public mapreduce::Reducer {
public:
    void reduce() override {
        mapreduce::key_t key;
        mapreduce::val_t val;

        while (get_next_pair(key, val)) {
            std::cerr << "[LONG REDUCER] key: " << key << ", val: " << val << std::endl;
            sleep(2);
            emit(key, val);
        }
    }
};

int main(int argc, char** argv) {
    sleep(10);
    std::cout << "[REDUCER WORKER] Starting binary" << std::endl;
    LongReducer reducer;

    reducer.start(argc, argv);
    
    return 0;
}