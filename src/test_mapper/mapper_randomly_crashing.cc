#include "../common/mapreduce.h"
#include "../common/utils.h"

#include "mapper_randomly_crashing.h"

int main(int argc, char** argv)
{
    MapperRandomlyCrashing mapper(7, 500);

    mapper.start(argc, argv);

    return 0;
}