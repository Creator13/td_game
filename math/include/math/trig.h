#pragma once
#include <cmath>

namespace math {
    constexpr float TWOPI = 6.2831853f;
    constexpr float PI = 3.1415927f;
    constexpr float HALFPI = 1.5707963f;

    constexpr float DEG2RAD = 0.017453293f;
    constexpr float RAD2DEG = 57.2957795f;

    inline float cos(float t) {
        return std::cos(t);
    }

    inline float sin(float t) {
        return std::sin(t);
    }

    inline float tan(float t) {
        return std::tan(t);
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

    inline float atan2(float y, float x) {
        return std::atan2(y, x);
    }
}
