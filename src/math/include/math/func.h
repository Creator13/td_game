#pragma once

#include <bit>
#include <cmath>

namespace math
{
    constexpr float EPSILON = 1e-5f;

    inline float sqrt(float val)
    {
        return std::sqrt(val);
    }

    template<typename T>
    constexpr T min(T a, T b)
    {
        return a < b ? a : b;
    }

    template<typename T>
    constexpr T max(T a, T b)
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

    inline float floor(float in)
    {
        return std::floor(in);
    }

    inline int floorToInt(float in)
    {
        return static_cast<int>(std::floor(in));
    }

    inline float ceil(float in)
    {
        return std::ceil(in);
    }

    inline int ceilToInt(float in)
    {
        return static_cast<int>(std::ceil(in));
    }

    inline float frac(float a)
    {
        return a - floor(a);
    }

    constexpr float lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    constexpr float clamp(float a, float min, float max)
    {
        if (a < min) return min;
        if (a > max) return max;
        return a;
    }

    constexpr float clamp01(float a)
    {
        return clamp(a, 0.0f, 1.0f);
    }

    constexpr float copysign(float num, float sgn) noexcept
    {
        uint32_t xi = std::bit_cast<uint32_t>(num) & 0x7fff'ffffu;
        uint32_t yi = std::bit_cast<uint32_t>(sgn) & 0x8000'0000u;
        return std::bit_cast<float>(xi | yi);
    }
}
