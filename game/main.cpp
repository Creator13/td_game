#include <iostream>

#include "math/vec.h"

int main() {
    using namespace math;

    vec2 a = vec2::zero;
    vec2 b = vec2::down;

    std::cout << "Distance: " << distance(a, b) << std::endl;
    std::cout << "Sqr distance: " << sqrDistance(a, b) << std::endl;
    return 0;
}
