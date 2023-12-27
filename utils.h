#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace mapreduce {

const std::string INTERMEDIATE_FILE_SUFFIX = "-intermediate.txt";

void split_file(std::string const& filepath, size_t part_size_mb);

} // mapreduce

#endif // UTILS_H