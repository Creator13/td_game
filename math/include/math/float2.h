#pragma once

#include "func.h"

namespace math {
    struct float2 {
        float x, y;

        constexpr float2() noexcept : x(0.0f), y(0.0f) { }
        constexpr float2(float x, float y) noexcept : x(x), y(y) { }

        constexpr float2& operator+=(const float2& rhs) noexcept;
        constexpr float2& operator-=(const float2& rhs) noexcept;
        constexpr float2& operator*=(float scalar) noexcept;
        constexpr float2& operator/=(float scalar) noexcept;
        constexpr bool operator==(const float2& rhs) const noexcept;
        constexpr bool operator!=(const float2& rhs) const noexcept;

        [[nodiscard]] float sqrLength() const noexcept;
        [[nodiscard]] float length() const noexcept;

        static const float2 zero, one;
        static const float2 up, down, left, right;
    };

    inline const float2 float2::zero{0.0f, 0.0f};
    inline const float2 float2::one{1.0f, 1.0f};
    inline const float2 float2::up{0.0f, 1.0f};
    inline const float2 float2::down{0.0f, -1.0f};
    inline const float2 float2::left{-1.0f, 0.0f};
    inline const float2 float2::right{1.0f, 0.0f};

    constexpr float2& float2::operator+=(const float2& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr float2& float2::operator-=(const float2& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr float2& float2::operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr float2& float2::operator/=(float scalar) noexcept {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    constexpr bool float2::operator==(const float2& rhs) const noexcept {
        return x == rhs.x && y == rhs.y;
    }

    constexpr bool float2::operator!=(const float2& rhs) const noexcept {
        return !operator==(rhs);
    }

    constexpr float2 operator+(float2 lhs, const float2& rhs) noexcept {
        return lhs += rhs;
    }

    constexpr float2 operator-(float2 lhs, const float2& rhs) noexcept {
        return lhs -= rhs;
    }

    constexpr float2 operator*(float2 vec, float scalar) noexcept {
        return vec *= scalar;
    }

    constexpr float2 operator*(float scalar, float2 vec) noexcept {
        return vec *= scalar;
    }

    constexpr float2 operator/(float2 vec, float scalar) noexcept {
        return vec /= scalar;
    }

    constexpr float2 operator/(float scalar, float2 vec) noexcept {
        return vec /= scalar;
    }

    constexpr float2 operator-(const float2& in) {
        return float2(-in.x, -in.y);
    }

    inline float float2::sqrLength() const noexcept {
        return x * x + y * y;
    }

    inline float float2::length() const noexcept {
        return sqrt(sqrLength());
    }

    inline float2 normalize(float2 in) noexcept {
        return in / in.length();
    }

    inline float sqrDistance(float2 from, float2 to) noexcept {
        return (to - from).sqrLength();
    }

    inline float distance(float2 from, float2 to) noexcept {
        return (to - from).length();
    }

    inline float dot(float2 lhs, float2 rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }

    inline float2 perp(float2 in) noexcept {
        return float2(in.y, -in.x);
    }

    constexpr float2 lerp(float2 a, float2 b, float t) noexcept {
        return a + (b - a) * t;
    }
}
