#pragma once
#include "vec2.h"

namespace math
{
    struct ivec2
    {
        int x, y;

        constexpr ivec2(int x, int y) : x(x), y(y) {}

        static constexpr ivec2 roundToInt(const vec2& v);
    };
}