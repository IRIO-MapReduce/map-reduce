#ifndef REDUCER_H
#define REDUCER_H

#include <vector>
#include <string>
#include <fstream>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"
#include "utils.h"

namespace mapreduce {

class Reducer {
public:
    /**
     * Used-defined reduce() function.
     * Should use get_next_pair() and emit() to read input and write output.
    */
    virtual void reduce() = 0;

    /**
     * Notifies the system that it is ready to start working, retrieves necessary info and runs reduce().
    */
    void start(int argc, char** argv);

protected:
    /**
     * Gets next key-value pair from input file.
     * To be used by custom reduce() implementation.
     * Returns false if there are no more pairs to be read.
    */
    bool get_next_pair(key_t& key, val_t& value);

    /**
     * Emits key-value pair the to intermediate file.
     * To be used by custom reduce() implementation.
    */
    void emit(key_t const& key, val_t const& value);

private:
    uint32_t group_id;
    uint32_t job_id;
    uint32_t num_mappers;
    std::string output_filepath;
    std::string input_filepath;
    std::string job_manager_address;
    std::string hash;
    std::fstream input_file;
    uint32_t current_mapper = 0;
};

} // mapreduce

#endif // REDUCER_H
