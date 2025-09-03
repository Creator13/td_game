#pragma once
#include "trig.h"

namespace math {
    // ################
    // **** float2 ****
    // ################

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

        static const float2 zero;
        static const float2 one;
        static const float2 up;
        static const float2 down;
        static const float2 left;
        static const float2 right;
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

    // ################
    // **** float3 ****
    // ################

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
    };

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

    // ################
    // **** float4 ****
    // ################

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
    };

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
}
