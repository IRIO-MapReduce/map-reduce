#include <sstream>

#include "../common/mapper.h"
#include "../common/mapreduce.h"
#include "../common/utils.h"

class WordCountMapper : public mapreduce::Mapper {
public:
    void map() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;

        while (get_next_pair(key, val)) {
            std::stringstream ss(val);
            std::string word;

            while (std::getline(ss, word, ' ')) {
                word.erase(std::remove_if(word.begin(), word.end(),
                               [](char c) { return !std::isalnum(c); }),
                    word.end());
                if (!word.empty())
                    emit(word, "1");
            }
        }
    }
};

int main(int argc, char** argv)
{
    WordCountMapper mapper;

    mapper.start(argc, argv);

    return 0;
}