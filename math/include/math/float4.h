#pragma once

#include "func.h"

namespace math {
    struct float4 {
        float x, y, z, w;

        constexpr float4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
        constexpr float4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) { }

        constexpr float4& operator+=(const float4& rhs) noexcept;
        constexpr float4& operator-=(const float4& rhs) noexcept;
        constexpr float4& operator*=(float scalar) noexcept;
        constexpr float4& operator/=(float scalar) noexcept;
        constexpr bool operator==(const float4& rhs) const noexcept;
        constexpr bool operator!=(const float4& rhs) const noexcept;

        [[nodiscard]] float sqrLength() const noexcept;
        [[nodiscard]] float length() const noexcept;

        static const float4 zero;
        static const float4 one;
    };

    inline const float4 float4::zero = float4(0.0f, 0.0f, 0.0f, 0.0f);;
    inline const float4 float4::one = float4(1.0f, 1.0f, 1.0f, 1.0f);;

    constexpr float4& float4::operator+=(const float4& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    constexpr float4& float4::operator-=(const float4& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    constexpr float4& float4::operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    constexpr float4& float4::operator/=(float scalar) noexcept {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    constexpr bool float4::operator==(const float4& rhs) const noexcept {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    constexpr bool float4::operator!=(const float4& rhs) const noexcept {
        return !operator==(rhs);
    }

    constexpr float4 operator+(float4 lhs, const float4& rhs) noexcept {
        return lhs += rhs;
    }

    constexpr float4 operator-(float4 lhs, const float4& rhs) noexcept {
        return lhs -= rhs;
    }

    constexpr float4 operator*(float4 vec, float scalar) noexcept {
        return vec *= scalar;
    }

    constexpr float4 operator*(float scalar, float4 vec) noexcept {
        return vec *= scalar;
    }

    constexpr float4 operator/(float4 vec, float scalar) noexcept {
        return vec /= scalar;
    }

    constexpr float4 operator/(float scalar, float4 vec) noexcept {
        return vec /= scalar;
    }

    constexpr float4 operator-(const float4& in) {
        return float4{-in.x, -in.y, -in.z, -in.w};
    }

    inline float float4::sqrLength() const noexcept {
        return x * x + y * y + z * z + w * w;
    }

    inline float float4::length() const noexcept {
        return sqrt(sqrLength());
    }

    inline float4 normalize(float4 in) noexcept {
        return in / in.length();
    }

    constexpr float dot(float4 lhs, float4 rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
    }

    constexpr float4 lerp(float4 a, float4 b, float t) noexcept {
        return a + (b - a) * t;
    }
}
