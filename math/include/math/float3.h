#pragma once

#include "func.h"

namespace math {
    struct float3 {
        float x, y, z;

        constexpr float3() noexcept : x(0.0f), y(0.0f), z(0.0f) { }
        constexpr float3(float x, float y, float z) noexcept : x(x), y(y), z(z) { }

        constexpr float3& operator+=(const float3& rhs) noexcept;
        constexpr float3& operator-=(const float3& rhs) noexcept;
        constexpr float3& operator*=(float scalar) noexcept;
        constexpr float3& operator/=(float scalar) noexcept;
        constexpr bool operator==(const float3& rhs) const noexcept;
        constexpr bool operator!=(const float3& rhs) const noexcept;

        [[nodiscard]] float sqrLength() const noexcept;
        [[nodiscard]] float length() const noexcept;

        static const float3 zero, one;
        static const float3 right, left, forward, backward, up, down;
    };

    inline const float3 float3::zero(0.0f, 0.0f, 0.0f);
    inline const float3 float3::one(1.f, 1.f, 1.f);
    inline const float3 float3::right(1.0f, 0.0f, 0.0f);
    inline const float3 float3::left(-1.0f, 0.0f, 0.0f);
    inline const float3 float3::forward(0.0f, 1.0f, 0.0f);
    inline const float3 float3::backward(0.0f, -1.0f, 0.0f);
    inline const float3 float3::up(0.0f, 0.0f, 1.0f);
    inline const float3 float3::down(0.0f, 0.0f, -1.0f);

    constexpr float3& float3::operator+=(const float3& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr float3& float3::operator-=(const float3& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr float3& float3::operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr float3& float3::operator/=(float scalar) noexcept {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    constexpr bool float3::operator==(const float3& rhs) const noexcept {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    constexpr bool float3::operator!=(const float3& rhs) const noexcept {
        return !operator==(rhs);
    }

    constexpr float3 operator+(float3 lhs, const float3& rhs) noexcept {
        return lhs += rhs;
    }

    constexpr float3 operator-(float3 lhs, const float3& rhs) noexcept {
        return lhs -= rhs;
    }

    constexpr float3 operator*(float3 vec, float scalar) noexcept {
        return vec *= scalar;
    }

    constexpr float3 operator*(float scalar, float3 vec) noexcept {
        return vec *= scalar;
    }

    constexpr float3 operator/(float3 vec, float scalar) noexcept {
        return vec /= scalar;
    }

    constexpr float3 operator/(float scalar, float3 vec) noexcept {
        return vec /= scalar;
    }

    constexpr float3 operator-(const float3& in) {
        return float3{-in.x, -in.y, -in.z};
    }

    inline float float3::sqrLength() const noexcept {
        return x * x + y * y + z * z;
    }

    inline float float3::length() const noexcept {
        return sqrt(sqrLength());
    }

    inline float3 normalize(float3 in) noexcept {
        return in / in.length();
    }

    inline float sqrDistance(float3 from, float3 to) noexcept {
        return (to - from).sqrLength();
    }

    inline float distance(float3 from, float3 to) noexcept {
        return (to - from).length();
    }

    constexpr float dot(float3 lhs, float3 rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }

    constexpr float3 cross(float3 lhs, float3 rhs) noexcept {
        return float3{
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x,
        };
    }

    constexpr float3 lerp(float3 a, float3 b, float t) noexcept {
        return a + (b - a) * t;
    }
}
