#include <iostream>

#include "testbed.h"

#define LOCAL

typedef void (*test_case_fun)();

const test_case_fun test_cases[] = {
    test_case_1,
    test_case_2,
    test_case_3,
    test_case_randomly_crashing,
    test_case_inter_write_crashing,
    test_case_huge,
    test_case_enormous,
    test_case_load
};
const int test_cases_num = 8;

void custom_test_case(std::string input, std::string output, std::string map_bin, std::string red_bin, int split_size, int num_reducers) {
    mapreduce::Config config;
    config.set_input_filepath(FS + input);
    config.set_output_filepath(FS + output);
    config.set_mapper_execpath(FS + map_bin);
    config.set_reducer_execpath(FS + red_bin);
    config.set_split_size_bytes(split_size);
    config.set_num_reducers(num_reducers);

    mapreduce::map_reduce(config);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return 0;
    }

    if (argc == 2) {
        int test_case_num = atoi(argv[1]) - 1;

        if (test_case_num < 0 || test_case_num >= test_cases_num) {
            std::cout << "test_case_num has to be in range [1, " << test_cases_num << "]" << std::endl;
        }

        test_cases[test_case_num]();
    }
    else if (argc == 7) {
        std::string input(argv[1]), output(argv[2]), map_bin(argv[3]), red_bin(argv[4]);
        int split_size = atoi(argv[5]), num_reducers = atoi(argv[6]);
        std::cerr << input << ' ' << output << ' ' << map_bin << ' ' << red_bin << ' ' << split_size << ' ' << num_reducers << std::endl;

        custom_test_case(input, output, map_bin, red_bin, split_size, num_reducers);
    }
    else {
        std::cout << "Usage: " << std::endl << 
                     "./testbed [test_case_num]" << std::endl << 
                     "./testbed [input filepath] [output filepath] [mapper binary] [reducer binary] [split size (bytes)] [num_reducers]" << std::endl;
    } 

    return 0;
}