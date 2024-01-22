#include "../common/mapreduce.h"
#include "../common/reducer.h"
#include "../common/utils.h"

#include "reducer_inter_write_crashing.h"

int main(int argc, char** argv)
{
    ReducerInterWriteCrashing reducer(7, 2, 500);

    reducer.start(argc, argv);

    return 0;
}