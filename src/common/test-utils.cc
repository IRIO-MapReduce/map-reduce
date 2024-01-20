#include "test-utils.h"

void wait_cpu(int n) {
    volatile int x;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            x += i % j;
        }
    }
}

int rand_in_range(int a, int b) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(a, b);
    return distr(gen);
}