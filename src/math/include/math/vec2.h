#pragma once

#include "func.h"

namespace math
{
    struct vec2
    {
        float x, y;

        constexpr vec2() noexcept : x(0.0f), y(0.0f) { }
        constexpr vec2(float x, float y) noexcept : x(x), y(y) { }

        constexpr vec2& operator+=(const vec2& rhs) noexcept;
        constexpr vec2& operator-=(const vec2& rhs) noexcept;
        constexpr vec2& operator*=(float scalar) noexcept;
        constexpr vec2& operator/=(float scalar) noexcept;
        constexpr bool operator==(const vec2& rhs) const noexcept;
        constexpr bool operator!=(const vec2& rhs) const noexcept;

        constexpr float sqrLength() const noexcept;
        MATH_CONSTEXPR_CMATH inline float length() const noexcept;

        static const vec2 zero, one;
        static const vec2 up, down, left, right;
    };

    inline const vec2 vec2::zero{0.0f, 0.0f};
    inline const vec2 vec2::one{1.0f, 1.0f};
    inline const vec2 vec2::up{0.0f, 1.0f};
    inline const vec2 vec2::down{0.0f, -1.0f};
    inline const vec2 vec2::left{-1.0f, 0.0f};
    inline const vec2 vec2::right{1.0f, 0.0f};

    constexpr vec2& vec2::operator+=(const vec2& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr vec2& vec2::operator-=(const vec2& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr vec2& vec2::operator*=(float scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr vec2& vec2::operator/=(float scalar) noexcept
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    constexpr bool vec2::operator==(const vec2& rhs) const noexcept
    {
        return x == rhs.x && y == rhs.y;
    }

    constexpr bool vec2::operator!=(const vec2& rhs) const noexcept
    {
        return !operator==(rhs);
    }

    constexpr bool approx(vec2 a, vec2 b, float epsilon = EPSILON)
    {
        return approx(a.x, b.x, epsilon) && approx(a.y, b.y, epsilon);
    }

    constexpr vec2 operator+(vec2 lhs, const vec2& rhs) noexcept
    {
        return lhs += rhs;
    }

    constexpr vec2 operator-(vec2 lhs, const vec2& rhs) noexcept
    {
        return lhs -= rhs;
    }

    constexpr vec2 operator*(vec2 vec, float scalar) noexcept
    {
        return vec *= scalar;
    }

    constexpr vec2 operator*(float scalar, vec2 vec) noexcept
    {
        return vec *= scalar;
    }

    constexpr vec2 operator/(vec2 vec, float scalar) noexcept
    {
        return vec /= scalar;
    }

    constexpr vec2 operator-(const vec2& in)
    {
        return vec2(-in.x, -in.y);
    }

    constexpr float vec2::sqrLength() const noexcept
    {
        return x * x + y * y;
    }

    MATH_CONSTEXPR_CMATH inline float vec2::length() const noexcept
    {
        return sqrt(sqrLength());
    }

    MATH_CONSTEXPR_CMATH inline vec2 normalize(vec2 in) noexcept
    {
        float sqrLength = in.sqrLength();
        return sqrLength > EPSILON ? in / sqrt(sqrLength) : vec2::zero;
    }

    constexpr float sqrDistance(vec2 from, vec2 to) noexcept
    {
        return (to - from).sqrLength();
    }

    MATH_CONSTEXPR_CMATH inline float distance(vec2 from, vec2 to) noexcept
    {
        return (to - from).length();
    }

    constexpr float dot(vec2 lhs, vec2 rhs) noexcept
    {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }

    constexpr vec2 perp(vec2 in) noexcept
    {
        return vec2(in.y, -in.x);
    }

    constexpr vec2 lerp(vec2 a, vec2 b, float t) noexcept
    {
        return a + (b - a) * t;
    }

    constexpr float max(vec2 in) noexcept
    {
        return max(in.x, in.y);
    }

    constexpr float min(vec2 in) noexcept
    {
        return min(in.x, in.y);
    }
}
