#pragma once
#include <cmath>

namespace math {
    constexpr float EPSILON = 0.00001f;

    constexpr bool approx(float a, float b, float epsilon) {
        return a - b < epsilon;
    }

    constexpr bool approx(float a, float b) {
        return approx(a, b, EPSILON);
    }

    template <typename T>
    constexpr T min(T a, T b) {
        return a < b ? a : b;
    }

    template <typename T>
    constexpr T max(T a, T b) {
        return a > b ? a : b;
    }

    template <typename T>
    T abs(T a) {
        return std::abs(a);
    }

    inline float floor(float in) {
        return std::floor(in);
    }

    inline int floorToInt(float in) {
        return static_cast<int>(std::floor(in));
    }

    inline float ceil(float in) {
        return std::ceil(in);
    }

    inline int ceilToInt(float in) {
        return static_cast<int>(std::ceil(in));
    }

    inline float frac(float a) {
        return a - floor(a);
    }

    constexpr float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
}
