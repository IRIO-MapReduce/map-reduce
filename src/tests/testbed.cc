#include <iostream>

#include "testbed.h"

typedef void (*test_case_fun) ();

const test_case_fun test_cases[] = {
    test_case_1,
    test_case_2,
    test_case_3,
    test_case_4,
    test_case_5
};

int main(int argc, char* argv[]) {

    if (argc != 1) {
        std::cout << "Usage: ./testbed [test_case_num]" << std::endl;
    }

    int test_case_num = atoi(argv[1]);

    test_cases[test_case_num]();

    return 0;
}