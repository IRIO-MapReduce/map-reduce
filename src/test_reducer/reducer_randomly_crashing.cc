#include "../common/mapreduce.h"
#include "../common/reducer.h"
#include "../common/utils.h"

#include "reducer_randomly_crashing.h"

int main(int argc, char** argv)
{
    ReducerRandomlyCrashing reducer(7, 500);

    reducer.start(argc, argv);

    return 0;
}