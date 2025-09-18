#pragma once

#include <cassert>

#include "func.h"

namespace math {
    struct vec4 {
        float x, y, z, w;

        constexpr vec4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
        constexpr vec4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) { }

        constexpr vec4& operator+=(const vec4& rhs) noexcept;
        constexpr vec4& operator-=(const vec4& rhs) noexcept;
        constexpr vec4& operator*=(float scalar) noexcept;
        constexpr vec4& operator/=(float scalar) noexcept;
        constexpr bool operator==(const vec4& rhs) const noexcept;
        constexpr bool operator!=(const vec4& rhs) const noexcept;

        [[nodiscard]] float sqrLength() const noexcept;
        [[nodiscard]] float length() const noexcept;

        static const vec4 zero;
        static const vec4 one;
    };

    inline const vec4 vec4::zero = vec4(0.0f, 0.0f, 0.0f, 0.0f);;
    inline const vec4 vec4::one = vec4(1.0f, 1.0f, 1.0f, 1.0f);;

    constexpr vec4& vec4::operator+=(const vec4& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    constexpr vec4& vec4::operator-=(const vec4& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    constexpr vec4& vec4::operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    constexpr vec4& vec4::operator/=(float scalar) noexcept {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    constexpr bool vec4::operator==(const vec4& rhs) const noexcept {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    constexpr bool vec4::operator!=(const vec4& rhs) const noexcept {
        return !operator==(rhs);
    }

    constexpr bool approx(vec4 a, vec4 b, float epsilon = EPSILON) {
        return approx(a.x, b.x, epsilon)
               && approx(a.y, b.y, epsilon)
               && approx(a.z, b.z, epsilon)
               && approx(a.w, b.w, epsilon);
    }

    constexpr vec4 operator+(vec4 lhs, const vec4& rhs) noexcept {
        return lhs += rhs;
    }

    constexpr vec4 operator-(vec4 lhs, const vec4& rhs) noexcept {
        return lhs -= rhs;
    }

    constexpr vec4 operator*(vec4 vec, float scalar) noexcept {
        return vec *= scalar;
    }

    constexpr vec4 operator*(float scalar, vec4 vec) noexcept {
        return vec *= scalar;
    }

    constexpr vec4 operator/(vec4 vec, float scalar) noexcept {
        return vec /= scalar;
    }

    constexpr vec4 operator-(const vec4& in) {
        return vec4{-in.x, -in.y, -in.z, -in.w};
    }

    inline float vec4::sqrLength() const noexcept {
        return x * x + y * y + z * z + w * w;
    }

    inline float vec4::length() const noexcept {
        return sqrt(sqrLength());
    }

    inline vec4 normalize(vec4 in) noexcept {
        float sqrLength = in.sqrLength();
        assert(sqrLength > EPSILON);
        return in / sqrt(sqrLength);
    }

    constexpr float dot(vec4 lhs, vec4 rhs) noexcept {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
    }

    constexpr vec4 lerp(vec4 a, vec4 b, float t) noexcept {
        return a + (b - a) * t;
    }
}
