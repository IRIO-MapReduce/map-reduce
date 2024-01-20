#include "../common/mapreduce.h"
#include "../common/utils.h"

#include "mapper_inter_write_crashing.h"

int main(int argc, char** argv) {
    std::cerr << "[MAPPER_INTER_WRITE_CRASHING WORKER] Starting binary" << std::endl;
    MapperInterWriteCrashing mapper(7, 5, 500);

    mapper.start(argc, argv);
    
    return 0;
}