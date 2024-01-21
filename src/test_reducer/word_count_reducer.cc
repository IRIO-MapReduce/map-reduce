#include <map>

#include "../common/mapreduce.h"
#include "../common/reducer.h"
#include "../common/utils.h"

class AddingReducer : public mapreduce::Reducer {
public:
    void reduce() override
    {
        mapreduce::key_t key;
        mapreduce::val_t val;
        std::map<mapreduce::key_t, long long> reduce_result;

        while (get_next_pair(key, val)) {
            reduce_result[key] += std::stoll(val);
        }
        for (auto const& [key, val] : reduce_result) {
            emit(key, std::to_string(val));
        }
    }
};

int main(int argc, char** argv)
{
    std::cerr << "[REDUCER WORKER] Starting binary" << std::endl;
    AddingReducer reducer;

    reducer.start(argc, argv);

    return 0;
}