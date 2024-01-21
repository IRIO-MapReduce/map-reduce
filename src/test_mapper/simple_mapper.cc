#include "../common/mapper.h"
#include "../common/mapreduce.h"
#include "../common/utils.h"

class SimpleMapper : public mapreduce::Mapper {
public:
    void map() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        while (get_next_pair(key, val)) {
            emit(key, val);
        }
    }
};

int main(int argc, char** argv)
{
    std::cerr << "[MAPPER WORKER] Starting binary" << std::endl;
    SimpleMapper mapper;

    mapper.start(argc, argv);

    return 0;
}