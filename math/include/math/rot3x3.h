#pragma once

#include "quaternion.h"
#include "vec3.h"

namespace math
{
    struct rot3x3
    {
        vec3 xBasis, yBasis, zBasis;

        constexpr rot3x3() noexcept { }

        constexpr rot3x3(vec3 x, vec3 y, vec3 z) noexcept : xBasis(x), yBasis(y), zBasis(z) { }

        // Format: m#row#col
        constexpr rot3x3(float m00, float m01, float m02,
                         float m10, float m11, float m12,
                         float m20, float m21, float m22)
            : xBasis(m00, m10, m20),
              yBasis(m01, m11, m21),
              zBasis(m02, m12, m22) { }

        constexpr bool operator==(const rot3x3& other) const;
        constexpr bool operator!=(const rot3x3& other) const;

        constexpr rot3x3& operator*=(const rot3x3& other);

        constexpr rot3x3 getTranspose() const;

        static constexpr rot3x3 fromQuaternion(quaternion q);
        static constexpr rot3x3 eulerAngles(float x, float y, float z);
        static rot3x3 lookRotation(const vec3& forward, const vec3& up);
        static rot3x3 angleAxis(float angle, const vec3& axis);

        static const rot3x3 identity;
    };

    const rot3x3 rot3x3::identity = rot3x3{
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };

    constexpr bool rot3x3::operator==(const rot3x3& other) const
    {
        return xBasis == other.xBasis && yBasis == other.yBasis && zBasis == other.zBasis;
    }

    constexpr bool rot3x3::operator!=(const rot3x3& other) const
    {
        return !operator==(other);
    }

    constexpr vec3 operator*(const rot3x3& lhs, vec3 vec)
    {
        return vec.x * lhs.xBasis + vec.y * lhs.yBasis + vec.z * lhs.zBasis;
    }

    constexpr rot3x3 operator*(const rot3x3& lhs, const rot3x3& rhs)
    {
        rot3x3 result;
        result.xBasis = lhs * rhs.xBasis;
        result.yBasis = lhs * rhs.yBasis;
        result.zBasis = lhs * rhs.zBasis;
        return result;
    }

    constexpr rot3x3& rot3x3::operator*=(const rot3x3& other)
    {
        *this = *this * other;
        return *this;
    }

    constexpr rot3x3 rot3x3::getTranspose() const
    {
        rot3x3 result;
        result.xBasis = vec3(this->xBasis.x, this->yBasis.x, this->zBasis.x);
        result.yBasis = vec3(this->xBasis.y, this->yBasis.y, this->zBasis.y);
        result.zBasis = vec3(this->xBasis.z, this->yBasis.z, this->zBasis.z);
        return result;
    }

    inline rot3x3 rot3x3::angleAxis(float angle, const vec3& axis)
    {
        if (angle == 0.0f) return identity;
        angle *= DEG2RAD;

        const vec3 norm = normalize(axis);
        const float s = sin(angle);
        const float c = cos(angle);

        // Shortcut rotation for world axes
        if (approx(norm, vec3::right)) // axis is aligned with world X
        {
            return rot3x3(
                vec3(1, 0, 0),
                vec3(0, c, s),
                vec3(0, -s, c)
            );
        }
        if (approx(norm, vec3::forward)) // Axis is aligned with world y
        {
            return rot3x3(
                vec3(c, 0, -s),
                vec3(0, 1, 0),
                vec3(s, 0, c)
            );
        }
        if (approx(norm, vec3::up)) // Axis is aligned with world Z
        {
            return rot3x3(
                vec3(c, s, 0),
                vec3(-s, c, 0),
                vec3(0, 0, 1)
            );
        }
        if (approx(norm, vec3::left)) // Axis is aligned with world -x
        {
            return rot3x3(
                vec3(-1, 0, 0),
                vec3(0, c, -s),
                vec3(0, s, c)
            );
        }
        if (approx(norm, vec3::back)) // Axis is aligned with world -y
        {
            return rot3x3(
                vec3(-c, 0, -s),
                vec3(0, -1, 0),
                vec3(s, 0, -c)
            );
        }
        if (approx(norm, vec3::down)) // Axis is aligned with world -Z
        {
            return rot3x3(
                vec3(c, -s, 0),
                vec3(s, c, 0),
                vec3(0, 0, -1)
            );
        }

        float x = norm.x;
        float y = norm.y;
        float z = norm.z;
        float t = 1.0 - c;

        vec3 xBasis = vec3(t * x * x + c, t * x * y - s * z, t * x * z + s * y);
        vec3 yBasis = vec3(t * x * y + s * z, t * y * y + c, t * y * z - s * x);
        vec3 zBasis = vec3(t * x * z - s * y, t * y * z + s * x, t * z * z + c);
        return rot3x3(xBasis, yBasis, zBasis);
    }

    inline rot3x3 rot3x3::lookRotation(const vec3& forward, const vec3& up)
    {
        vec3 f = normalize(forward); // local y
        vec3 r = normalize(cross(f, up)); // local x
        vec3 u = cross(r, f); // local z
        return rot3x3(r, f, u);
    }

    constexpr rot3x3 orthonormalize(const rot3x3& in)
    {
        vec3 x = normalize(in.xBasis);

        vec3 y = in.yBasis - x * dot(in.yBasis, x);
        if (in.yBasis.sqrLength() < EPSILON)
        {
            y = math::abs(x.x) > math::abs(x.z) ? vec3(-x.y, x.x, 0) : vec3(0, -x.z, x.y);
        }
        y = normalize(y);

        vec3 z = cross(x, y);

        return rot3x3(x, y, z);
    }
}
