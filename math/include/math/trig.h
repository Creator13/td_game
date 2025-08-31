#pragma once
#include <cmath>

namespace math {
    constexpr float PI = 3.1415927f;
    constexpr float DEG2RAD = 0.017453293f;
    constexpr float RAD2DEG = 57.2957795f;

    inline float cos(float t) {
        return std::cosf(t);
    }

    inline float sin(float t) {
        return std::sinf(t);
    }

    inline float tan(float t) {
        return std::tanf(t);
    }

    inline float asin(float t) {
        return std::asin(t);
    }

    inline float acos(float t) {
        return std::acos(t);
    }

    inline float atan(float t) {
        return std::atan(t);
    }

    inline float sqrt(float t) {
        return std::sqrtf(t);
    }
}
