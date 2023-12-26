#include "mapreduce.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

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
    void reduce(std::string const& filepath) const override {
        std::ifstream file(filepath);
        std::string line;
        std::unordered_map<std::string, int> counts;
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string key, value;

            std::getline(ss, key, ',');
            std::getline(ss, value);

            counts[key] += std::stoi(value);
        }

        for (auto const& [key, value] : counts) {
            emit(key, std::to_string(value));
        }
    }
};

int main() {
    mapreduce::Config config;
    config.set_input_file("input.txt");
    config.set_output_file("output.txt");

    CounterMapper mapper;
    CounterReducer reducer;

    config.set_mapper(&mapper);
    config.set_reducer(&reducer);

    mapreduce::map_reduce(config);

    return 0;
}