#include <fstream>
#include <sstream>
#include <unordered_map>

#include "mapreduce.h"
#include "utils.h"

class CounterMapper : public mapreduce::Mapper {
public:
    void map(std::string const& filepath) const override {
        std::ifstream file(filepath);
        std::string line;
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string key, value;

            std::getline(ss, key, ',');
            std::getline(ss, value);

            emit(key, "1");
        }
    }
};

class CounterReducer : public mapreduce::Reducer {
public:
    void reduce(std::vector<std::string> const& filepaths) const override {
        std::unordered_map<std::string, int> counts;

        for (auto const& filepath : filepaths) {
            std::ifstream file(filepath);
            std::string line;
            
            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string key, value;

                std::getline(ss, key, ',');
                std::getline(ss, value);

                counts[key] += std::stoi(value);
            }
        }
        
        for (auto const& [key, value] : counts) {
            emit(key, std::to_string(value));
        }
    }
};

int main() {
    mapreduce::Config config;
    config.set_input_file("tests/input2.txt");
    config.set_output_file("tests/output2.txt");

    CounterMapper mapper;
    CounterReducer reducer;

    config.set_mapper(&mapper);
    config.set_reducer(&reducer);

    config.set_split_size(1);

    mapreduce::map_reduce(config);

    return 0;
}