#pragma once
#include "math/mat4.h"

namespace constants
{
    // Define a z+ up, y+ forward coordinate system
    constexpr math::mat4 COORDINATE_BASIS = math::mat4(
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, -1, 0, 0,
        0, 0, 0, 1
    );
}