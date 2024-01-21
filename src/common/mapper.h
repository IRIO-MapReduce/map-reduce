#ifndef MAPPER_H
#define MAPPER_H

#include <fstream>
#include <grpc++/grpc++.h>
#include <string>

#include "mapreduce.grpc.pb.h"
#include "utils.h"

namespace mapreduce {

class Mapper {
public:
    /**
     * Used-defined map() function.
     * Should use get_next_pair() and emit() to read input and write output.
     */
    virtual void map() = 0;

    /**
     * Notifies the system that it is ready to start working, retrieves
     * necessary info and runs map().
     */
    void start(int argc, char** argv);

protected:
    /**
     * Gets next key-value pair from input file.
     * To be used by custom map() implementation.
     * Returns false if there are no more pairs to be read.
     */
    bool get_next_pair(key_t& key, val_t& value);

    /**
     * Emits key-value pair the to intermediate file.
     * To be used by custom map() implementation.
     */
    void emit(key_t const& key, val_t const& value);

private:
    /**
     * Opens input and output files. Should be invoked once per map(). Speeds up
     * writing and reading.
     */
    void open_files();

    /**
     * Closes input and output files. Should be invoked once per map(). Speeds
     * up writing and reading.
     */
    void close_files();

private:
    uint32_t group_id;
    uint32_t job_id;
    uint32_t num_reducers;
    std::string input_filepath;
    std::string job_manager_address;
    std::string hash;
    std::fstream input_file;
    std::vector<std::ofstream> output_files;
};

} // mapreduce

#endif // MAPPER_H
