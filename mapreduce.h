#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <string>
#include <vector>

namespace mapreduce {

constexpr size_t DEFAULT_SPLIT_SIZE_MB = 32;

using key_t = std::string;
using value_t = std::string;

class Worker {
public:
    void set_emit_file(std::string const& filepath);

protected:
    void emit(key_t const& key, value_t const& value) const;

private:
    std::string output_filepath;
};

class Mapper : public Worker {
public:
    virtual void map(std::string const& filepath) const = 0;
};

class Reducer : public Worker {
public:
    virtual void reduce(std::vector<std::string> const& filepaths) const = 0;
};

class Config {
public:
    void set_input_file(std::string const& filepath);
    void set_output_file(std::string const& filepath);
    void set_mapper(Mapper *mapper);
    void set_reducer(Reducer *reducer);
    void set_split_size(size_t split_size_mb);

private:
    std::string input_filepath;
    std::string output_filepath;
    Mapper *mapper;
    Reducer *reducer;
    size_t split_size_mb;

    friend void map_reduce(Config const& config);
};

void map_reduce(Config const& config);

} // mapreduce

#endif // MAP_REDUCE_H