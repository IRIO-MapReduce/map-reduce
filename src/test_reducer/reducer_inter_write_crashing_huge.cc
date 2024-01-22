#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/reducer.h"

#include "reducer_inter_write_crashing.h"

int main(int argc, char** argv) {
    ReducerInterWriteCrashing reducer(8, 3, 5000);

    reducer.start(argc, argv);
    
    return 0;
}