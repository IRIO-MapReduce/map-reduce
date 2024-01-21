#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/reducer.h"

#include "reducer_randomly_crashing.h"

int main(int argc, char** argv) {
    std::cerr << "[REDUCER_RANDOMLY_CRASHING WORKER] Starting binary" << std::endl;
    ReducerRandomlyCrashing reducer(7, 500);

    reducer.start(argc, argv);
    
    return 0;
}