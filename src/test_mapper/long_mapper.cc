#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/mapper.h"

class LongMapper : public mapreduce::Mapper {
public:
    void map() override {
        mapreduce::key_t key;
        mapreduce::val_t val;

        while (get_next_pair(key, val)) {
            std::cerr << "[LONG MAPPER] key: " << key << ", val: " << val << std::endl;
            sleep(2);
            emit(key, val);
        }
    }
};

int main(int argc, char** argv) {
    std::cerr << "[MAPPER WORKER] Starting binary" << std::endl;
    LongMapper mapper;

    mapper.start(argc, argv);
    
    return 0;
}