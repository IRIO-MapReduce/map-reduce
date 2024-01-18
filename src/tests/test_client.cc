#include "../common/mapreduce.h"
#include "../common/utils.h"

int main() {
    // Set up config before running!!
    mapreduce::Config config;
    config.set_input_filepath("../../fs/input.txt");
    config.set_output_filepath("../../fs/output.txt");
    config.set_mapper_execpath("../../test_mapper/build/simple_mapper");
    config.set_reducer_execpath("../../test_reducer/build/simple_reducer");
    config.set_split_size_bytes(1);
    config.set_num_reducers(3);

    mapreduce::map_reduce(config);

    return 0;
}