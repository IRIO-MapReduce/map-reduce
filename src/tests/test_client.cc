#include "../common/mapreduce.h"
#include "../common/utils.h"

int main() {
    // Set up config before running!!
    mapreduce::Config config;
    config.set_input_filepath("/mnt/fs/input.txt");
    config.set_output_filepath("/mnt/fs/output.txt");
    config.set_mapper_execpath("/mnt/fs/simple_mapper");
    config.set_reducer_execpath("/mnt/fs/simple_reducer");
    config.set_split_size_bytes(1);
    config.set_num_reducers(3);

    mapreduce::map_reduce(config);

    return 0;
}
