#include <iostream>

#include "../common/mapreduce.h"
#include "../common/utils.h"

int main(int argc, char** argv) {
    if (argc != 7) {
        std::cerr << "[ERROR] Usage: ./client <mapper> <reducer> <input_file> <output_file> <split size> <number of reducers>" << std::endl;
        return 1;
    }

    std::string mapper_execpath(argv[1]);
    std::string reducer_execpath(argv[2]);
    std::string input_filepath(argv[3]);
    std::string output_filepath(argv[4]);
    size_t split_size_bytes = std::stoi(argv[5]);
    size_t num_reducers = std::stoi(argv[6]);

    // Set up config before running!!
    mapreduce::Config config;
    config.set_input_filepath(input_filepath);
    config.set_output_filepath(output_filepath);
    config.set_mapper_execpath(mapper_execpath);
    config.set_reducer_execpath(reducer_execpath);
    config.set_split_size_bytes(split_size_bytes);
    config.set_num_reducers(num_reducers);

    mapreduce::map_reduce(config);

    return 0;
}