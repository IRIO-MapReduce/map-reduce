#include <iostream>

#include "../common/test-utils.h"

int main(int argc, char* argv[]) {
    std::cerr << "Usage: ./gen_synthetic_test [keys] [lines]" << std::endl;
    int keys = atoi(argv[1]);
    int lines = atoi(argv[2]);
    std::cerr << "Generating test with " << keys << "keys and " << lines << " lines" << std::endl;
    
    for (int i = 0; i < lines; i++) {
        int key = rand_in_range(0, keys - 1);
        int value = rand_in_range(0, 9);
        std::cout << key << "," << value << std::endl;
    }
}