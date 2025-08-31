#include <iostream>

#include "math/vec.h"

int main() {
    using namespace math;

    float2 a = float2::zero;
    float2 b = float2::down;

    std::cout << "Distance: " << float2::distance(a, b) << std::endl;
    std::cout << "Sqr distance: " << float2::sqrDistance(a, b) << std::endl;
    return 0;
}
