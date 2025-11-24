#pragma once

#include "vec2.h"
#include "func.h"

namespace math
{
    struct vec3
    {
        float x, y, z;

        constexpr vec3() noexcept : x(0.0f), y(0.0f), z(0.0f) { }
        constexpr vec3(float x, float y, float z) noexcept : x(x), y(y), z(z) { }
        constexpr vec3(vec2 vec, float z) noexcept : x(vec.x), y(vec.y), z(z) { }

        constexpr vec3& operator+=(const vec3& rhs) noexcept;
        constexpr vec3& operator-=(const vec3& rhs) noexcept;
        constexpr vec3& operator*=(float scalar) noexcept;
        constexpr vec3& operator/=(float scalar) noexcept;
        constexpr bool operator==(const vec3& rhs) const noexcept;
        constexpr bool operator!=(const vec3& rhs) const noexcept;

        constexpr float sqrLength() const noexcept;
        [[nodiscard]] float length() const noexcept;

        constexpr vec2 xy() const noexcept;
        constexpr vec2 xz() const noexcept;
        constexpr vec2 yz() const noexcept;

        static const vec3 zero, one;
        static const vec3 right, left, forward, back, up, down;
    };

    inline const vec3 vec3::zero(0.0f, 0.0f, 0.0f);
    inline const vec3 vec3::one(1.f, 1.f, 1.f);

    inline const vec3 vec3::right(1.0f, 0.0f, 0.0f);
    inline const vec3 vec3::left(-1.0f, 0.0f, 0.0f);

    inline const vec3 vec3::forward(0.0f, 1.0f, 0.0f);
    inline const vec3 vec3::back(0.0f, -1.0f, 0.0f);

    inline const vec3 vec3::up(0.0f, 0.0f, 1.0f);
    inline const vec3 vec3::down(0.0f, 0.0f, -1.0f);

    constexpr vec3& vec3::operator+=(const vec3& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr vec3& vec3::operator-=(const vec3& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr vec3& vec3::operator*=(float scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr vec3& vec3::operator/=(float scalar) noexcept
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    constexpr bool vec3::operator==(const vec3& rhs) const noexcept
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    constexpr bool vec3::operator!=(const vec3& rhs) const noexcept
    {
        return !operator==(rhs);
    }

    constexpr vec2 vec3::xy() const noexcept { return vec2(x, y); }
    constexpr vec2 vec3::xz() const noexcept { return vec2(x, z); }
    constexpr vec2 vec3::yz() const noexcept { return vec2(y, z); }

    constexpr bool approx(vec3 a, vec3 b, float epsilon = EPSILON)
    {
        return approx(a.x, b.x, epsilon)
               && approx(a.y, b.y, epsilon)
               && approx(a.z, b.z, epsilon);
    }

    constexpr vec3 operator+(vec3 lhs, const vec3& rhs) noexcept
    {
        return lhs += rhs;
    }

    constexpr vec3 operator-(vec3 lhs, const vec3& rhs) noexcept
    {
        return lhs -= rhs;
    }

    constexpr vec3 operator*(vec3 vec, float scalar) noexcept
    {
        return vec *= scalar;
    }

    constexpr vec3 operator*(float scalar, vec3 vec) noexcept
    {
        return vec *= scalar;
    }

    constexpr vec3 operator/(vec3 vec, float scalar) noexcept
    {
        return vec /= scalar;
    }

    constexpr vec3 operator-(const vec3& in)
    {
        return vec3{-in.x, -in.y, -in.z};
    }

    constexpr float vec3::sqrLength() const noexcept
    {
        return x * x + y * y + z * z;
    }

    inline float vec3::length() const noexcept
    {
        return sqrt(sqrLength());
    }

    inline vec3 normalize(vec3 in) noexcept
    {
        float sqrLength = in.sqrLength();
        return sqrLength > EPSILON ? in / sqrt(sqrLength) : vec3::zero;
    }

    constexpr float sqrDistance(vec3 from, vec3 to) noexcept
    {
        return (to - from).sqrLength();
    }

    inline float distance(vec3 from, vec3 to) noexcept
    {
        return (to - from).length();
    }

    constexpr float dot(vec3 lhs, vec3 rhs) noexcept
    {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }

    constexpr vec3 cross(vec3 lhs, vec3 rhs) noexcept
    {
        return vec3{
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x,
        };
    }

    constexpr vec3 compMul(vec3 lhs, vec3 rhs) noexcept
    {
        return vec3{
            lhs.x * rhs.x,
            lhs.y * rhs.y,
            lhs.z * rhs.z
        };
    }

    constexpr vec3 lerp(vec3 a, vec3 b, float t) noexcept
    {
        return a + (b - a) * t;
    }

    constexpr float max(vec3 in) noexcept
    {
        return max(in.x, max(in.y, in.z));
    }

    constexpr float min(vec3 in) noexcept
    {
        return min(in.x, min(in.y, in.z));
    }
}
