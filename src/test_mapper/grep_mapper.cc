#include "../common/mapper.h"
#include "../common/mapreduce.h"
#include "../common/utils.h"

class LongMapper : public mapreduce::Mapper {
public:
    void map() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        std::string word = "MapReduce";

        while (get_next_pair(key, val)) {
            if (val.find(word) != std::string::npos) {
                emit(key, val);
            }
        }
    }
};

int main(int argc, char** argv)
{
    std::cerr << "[MAPPER WORKER] Starting binary" << std::endl;
    LongMapper mapper;

    mapper.start(argc, argv);

    return 0;
}