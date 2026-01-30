#pragma once

#include <bit>
#include <cmath>
#include <cstdint>

#include "math/_platform.h"

namespace math
{
    constexpr float EPSILON = 1e-5f;

    MATH_CONSTEXPR_CMATH inline float sqrt(float val)
    {
        return std::sqrt(val);
    }

    constexpr auto min(auto a, auto b)
    {
        return a < b ? a : b;
    }

    constexpr auto max(auto a, auto b)
    {
        return a > b ? a : b;
    }

    constexpr float abs(float x) noexcept
    {
        // clear the sign-bit of the IEEE-754 32-bit float
        uint32_t bits = std::bit_cast<uint32_t>(x);
        bits &= 0x7FFF'FFFFu;
        return std::bit_cast<float>(bits);
    }

    constexpr bool approx(float a, float b, float epsilon = EPSILON)
    {
        return abs(a - b) < epsilon;
    }

    MATH_CONSTEXPR_CMATH inline float floor(float in)
    {
        return std::floor(in);
    }

    MATH_CONSTEXPR_CMATH inline int floorToInt(float in)
    {
        return static_cast<int>(std::floor(in));
    }

    MATH_CONSTEXPR_CMATH inline float ceil(float in)
    {
        return std::ceil(in);
    }

    MATH_CONSTEXPR_CMATH inline int ceilToInt(float in)
    {
        return static_cast<int>(std::ceil(in));
    }

    MATH_CONSTEXPR_CMATH inline float frac(float a)
    {
        return a - floor(a);
    }

    MATH_CONSTEXPR_CMATH inline float exp(float a)
    {
        return std::exp(a);
    }

    MATH_CONSTEXPR_CMATH inline float log2(float a)
    {
        return std::log2(a);
    }

    MATH_CONSTEXPR_CMATH inline float pow(float a, float b)
    {
        return std::pow(a, b);
    }

    constexpr float lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    constexpr float clamp(float a, float min, float max)
    {
        if (a <= min) return min;
        if (a >= max) return max;
        return a;
    }

    constexpr float clamp01(float a)
    {
        return clamp(a, 0.0f, 1.0f);
    }

    constexpr float copysign(float num, float sgn) noexcept
    {
        const uint32_t xi = std::bit_cast<uint32_t>(num) & 0x7fff'ffffu;
        const uint32_t yi = std::bit_cast<uint32_t>(sgn) & 0x8000'0000u;
        return std::bit_cast<float>(xi | yi);
    }
}
