#ifndef MAPPER_H
#define MAPPER_H

#include <string>
#include <fstream>
#include <grpc++/grpc++.h>

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
     * Notifies the system that it is ready to start working, retrieves necessary info and runs map().
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
    uint32_t group_id;
    uint32_t job_id;
    uint32_t num_reducers;
    std::string input_filepath;
    std::fstream input_file;
};

} // mapreduce

#endif // MAPPER_H
