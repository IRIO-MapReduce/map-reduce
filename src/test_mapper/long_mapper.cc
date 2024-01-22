#include "../common/mapper.h"
#include "../common/mapreduce.h"
#include "../common/utils.h"

class LongMapper : public mapreduce::Mapper {
public:
    void map() override
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
    LongMapper mapper;

    mapper.start(argc, argv);

    return 0;
}