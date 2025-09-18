#pragma once

#include <assert.h>

#include "rot3x3.h"
#include "vec3.h"
#include "vec4.h"
#include "trig.h"

namespace math
{
    struct quaternion
    {
        float x, y, z, w;

        constexpr quaternion() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
        constexpr quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { };

        explicit constexpr operator vec4() const noexcept { return vec4(x, y, z, w); }
        constexpr explicit quaternion(const vec4& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) { }

        constexpr static quaternion inverse(quaternion q);

        /// Creates a rotation by degrees around each axis, in XYZ order
        static quaternion eulerAngles(float x, float y, float z);
        static quaternion lookRotation(const vec3& forward, const vec3& up);
        static quaternion angleAxis(float angle, const vec3& axis);

        static quaternion fromRot3x3(const rot3x3& rotMatrix);

        static const quaternion identity;
    };

    constexpr quaternion operator*(const quaternion& lhs, const quaternion& rhs) noexcept;

    inline quaternion normalize(quaternion q);
    constexpr float dot(quaternion a, quaternion b);
    inline vec3 toEuler(quaternion q);
    inline quaternion slerp(quaternion a, quaternion b, float t);

    constexpr quaternion quaternion::identity = quaternion(0, 0, 0, 1);

    constexpr quaternion quaternion::inverse(quaternion q)
    {
        // Actually computes the conjugate of the quaternion, not the true inverse, on the basis that we always work with a unit quaternion.
        // inverse(q) = conjugate(q) / magnitude(q) -> if magnitude is 1 (unit quaternion), the inverse is simply the conjugate
        // Does not normalize as we may assume the caller provides a valid unit quaternion, and the unary minus does not introduce new drift.
        return quaternion(-q.x, -q.y, -q.z, q.w);
    }

    inline quaternion quaternion::eulerAngles(float x, float y, float z)
    {
        x *= DEG2RAD;
        y *= DEG2RAD;
        z *= DEG2RAD;

        const float sx = sin(x * .5f);
        const float cx = cos(x * .5f);
        const float sy = sin(y * .5f);
        const float cy = cos(y * .5f);
        const float sz = sin(z * .5f);
        const float cz = cos(z * .5f);

        quaternion q;
        q.x = sx * cy * cz + cx * sy * sz;
        q.y = cx * sy * cz - sx * cy * sz;
        q.z = cx * cy * sz + sx * sy * cz;
        q.w = cx * cy * cz - sx * sy * sz;
        return normalize(q);
    }

    inline quaternion quaternion::lookRotation(const vec3& forward, const vec3& up)
    {
        return fromRot3x3(rot3x3::lookRotation(forward, up));
    }

    inline quaternion quaternion::angleAxis(float angle, const vec3& axis)
    {
        if (angle == 0)
        {
            return identity;
        }

        const vec3 norm = normalize(axis);
        const float halfAngle = angle * DEG2RAD * 0.5f;
        const float s = sin(halfAngle);

        return quaternion(norm.x * s, norm.y * s, norm.z * s, cos(halfAngle));
    }

    inline quaternion quaternion::fromRot3x3(const rot3x3& m)
    {
        // @formatter:off
        float m11 = m.xBasis.x; float m12 = m.yBasis.x; float m13 = m.zBasis.x;
        float m21 = m.xBasis.y; float m22 = m.yBasis.y; float m23 = m.zBasis.y;
        float m31 = m.xBasis.z; float m32 = m.yBasis.z; float m33 = m.zBasis.z;
        // @formatter:on

        float fourWSqMinus1 = m11 + m22 + m33;
        float fourXSqMinus1 = m11 - m22 - m33;
        float fourYSqMinus1 = m22 - m11 - m33;
        float fourZSqMinus1 = m33 - m11 - m22;

        // Determine which of w, x, y, z has the largest value
        int biggestIndex = 0;
        float fourBiggestSqMinus1 = fourWSqMinus1;
        if (fourXSqMinus1 > fourBiggestSqMinus1)
        {
            fourBiggestSqMinus1 = fourXSqMinus1;
            biggestIndex = 1;
        }
        if (fourYSqMinus1 > fourBiggestSqMinus1)
        {
            fourBiggestSqMinus1 = fourYSqMinus1;
            biggestIndex = 2;
        }
        if (fourZSqMinus1 > fourBiggestSqMinus1)
        {
            fourBiggestSqMinus1 = fourZSqMinus1;
            biggestIndex = 3;
        }

        float biggestVal = sqrt(fourBiggestSqMinus1 + 1.f) * .5f;
        float mult = .25f / biggestVal;

        // Compute quaternion
        quaternion q;
        switch (biggestIndex)
        {
            case 0:
                q.w = biggestVal;
                q.x = (m23 - m32) * mult;
                q.y = (m31 - m13) * mult;
                q.z = (m12 - m21) * mult;
                break;
            case 1:
                q.x = biggestVal;
                q.w = (m23 - m32) * mult;
                q.y = (m12 + m21) * mult;
                q.z = (m31 + m13) * mult;
                break;
            case 2:
                q.y = biggestVal;
                q.w = (m31 - m13) * mult;
                q.x = (m12 + m21) * mult;
                q.z = (m23 + m32) * mult;
                break;
            case 3:
                q.z = biggestVal;
                q.w = (m12 - m21) * mult;
                q.x = (m31 + m13) * mult;
                q.y = (m23 - m32) * mult;
                break;
        }
        return normalize(q);
    }

    // ****
    // FREE FUNCTIONS
    // ****

    constexpr quaternion operator*(const quaternion& lhs, const quaternion& rhs) noexcept
    {
        quaternion result;
        result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
        result.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
        result.y = lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z;
        result.z = lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x;
        return normalize(result);
    }

    inline quaternion normalize(quaternion q)
    {
        const float square = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
        assert(square > EPSILON); // Panic from zero-quaternion (should never occur)
        const float inv = 1.f / sqrt(square);
        return quaternion(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
    }

    constexpr float dot(quaternion a, quaternion b)
    {
        return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline vec3 toEuler(quaternion q)
    {
        vec3 euler;
        float sinp = 2 * (q.w * q.x - q.y * q.z);
        sinp = clamp(sinp, -1.f, 1.f);

        if (approx(abs(sinp), 1))
        {
            euler.x = HALFPI * sinp;
            euler.y = atan2(2 * (q.w * q.y + q.z * q.x), 1 - 2 * (q.x * q.x + q.y * q.y));
            euler.z = 0;
        }
        else
        {
            euler.x = asin(sinp);
            euler.y = atan2(2 * (q.w * q.y + q.z * q.x), 1 - 2 * (q.x * q.x + q.y * q.y));
            euler.z = atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.y * q.y + q.z * q.z));
        }

        return euler * RAD2DEG;
    }

    inline quaternion slerp(quaternion a, quaternion b, float t)
    {
        return quaternion::identity;
    }
}
