#include "../common/mapreduce.h"
#include "../common/utils.h"

#ifndef LOCAL
const std::string FS = "/mnt/fs/";
#else
const std::string FS = "../../fs/";
#endif

int main()
{
    mapreduce::Config config;
    config.set_input_filepath(FS + "input.txt");
    config.set_output_filepath(FS + "output.txt");
    config.set_mapper_execpath(FS + "simple_mapper");
    config.set_reducer_execpath(FS + "simple_reducer");
    config.set_split_size_bytes(1);
    config.set_num_reducers(3);

    mapreduce::map_reduce(config);

    return 0;
}
