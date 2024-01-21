#include <iostream>

#include "testbed.h"

typedef void (*test_case_fun) ();

const test_case_fun test_cases[] = {
    test_case_1,
    test_case_2,
    test_case_3,
    test_case_randomly_crashing,
    test_case_inter_write_crashing,
    test_case_huge
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./testbed [test_case_num]" << std::endl;
        return 0;
    }

    int test_case_num = atoi(argv[1]) - 1;

    if (test_case_num < 1 || test_case_num > 6) {
        std::cout << "test_case_num has to be in range [1, 6]" << std::endl;
    }

    test_cases[test_case_num]();

    return 0;
}