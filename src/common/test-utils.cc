#include "test-utils.h"

#include <chrono>
#include <iostream>

void wait_cpu(int n)
{
    volatile int x;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            x += i % j;
        }
    }
}

int rand_in_range(int a, int b)
{
    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    int result = rand() % (b - a + 1) + a;
    std::cerr << result << std::endl;
    return result;
}