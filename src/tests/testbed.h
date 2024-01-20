#ifndef TESTBED_H
#define TESTBED_H

#include "../common/mapreduce.h"
#include "../common/utils.h"

#ifndef LOCAL
const std::string FS = "/mnt/fs/";
#else
const std::string FS = "../../fs/";
#endif

void test_case_1() {
    mapreduce::Config config;
    config.set_input_filepath(FS + "input.txt");
    config.set_output_filepath(FS + "output.txt");
    config.set_mapper_execpath(FS + "simple_mapper");
    config.set_reducer_execpath(FS + "simple_reducer");
    config.set_split_size_bytes(1);
    config.set_num_reducers(3);

    mapreduce::map_reduce(config);
}

void test_case_2() {
    mapreduce::Config config;
    config.set_input_filepath(FS + "input-big.txt");
    config.set_output_filepath(FS + "output-big.txt");
    config.set_mapper_execpath(FS + "simple_mapper");
    config.set_reducer_execpath(FS + "simple_reducer");
    config.set_split_size_bytes(1);
    config.set_num_reducers(6);

    mapreduce::map_reduce(config);
}

void test_case_3() {
    mapreduce::Config config;
    config.set_input_filepath(FS + "input-big.txt");
    config.set_output_filepath(FS + "output-big.txt");
    config.set_mapper_execpath(FS + "long_mapper");
    config.set_reducer_execpath(FS + "long_reducer");
    config.set_split_size_bytes(1);
    config.set_num_reducers(3);

    mapreduce::map_reduce(config);
}

void test_case_4() {
    mapreduce::Config config;
    config.set_input_filepath(FS + "input-big.txt");
    config.set_output_filepath(FS + "output-big.txt");
    config.set_mapper_execpath(FS + "mapper_randomly_crashing");
    config.set_reducer_execpath(FS + "reducer_randomly_crashing");
    config.set_split_size_bytes(1);
    config.set_num_reducers(6);

    mapreduce::map_reduce(config);
}

void test_case_5() {
    mapreduce::Config config;
    config.set_input_filepath(FS + "input-big.txt");
    config.set_output_filepath(FS + "output-big.txt");
    config.set_mapper_execpath(FS + "mapper_inter_write_crashing");
    config.set_reducer_execpath(FS + "reducer_inter_write_crashing");
    config.set_split_size_bytes(1);
    config.set_num_reducers(6);

    mapreduce::map_reduce(config);
}

#endif // TESTBED_H
