#include <iostream>

namespace psr {
    int randomBetween(int min, int max) {
        unsigned int seed = static_cast<unsigned int>(time(nullptr));
        seed ^= reinterpret_cast<unsigned int>(&seed) >> 10;
        unsigned int result = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return min + (seed % (max - min + 1));
    }
    float randomBetweenFloat(double min, double max) {
        unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
        seed ^= reinterpret_cast<unsigned int>(&seed) >> 10;
        unsigned int result = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        double random = min + (result % static_cast<unsigned int>((max - min) * 10000)) / 10000.0;
        return random;
    }
}
