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
    req_id_t id;
    std::string output_filepath;
    std::vector<std::string> input_filepaths;
    std::fstream input_file;
};

} // mapreduce

#endif // REDUCER_H
