#pragma once

#include <assert.h>

#include "vec3.h"
#include "vec4.h"
#include "trig.h"

namespace math
{
    struct rot3x3;

    struct quaternion
    {
        float x, y, z, w;

        constexpr quaternion() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
        constexpr quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { };

        explicit constexpr operator vec4() const noexcept { return vec4(x, y, z, w); }
        constexpr explicit quaternion(const vec4& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) { }

        constexpr bool operator==(const quaternion&) const;
        constexpr bool operator!=(const quaternion&) const;

        /// Creates a rotation by degrees around each axis, in XYZ order
        static quaternion eulerAngles(float x, float y, float z);
        static quaternion eulerAngles(vec3 angles);
        static quaternion lookRotation(const vec3& forward, const vec3& up);
        static quaternion angleAxis(float angle, const vec3& axis);

        static quaternion fromRot3x3(const rot3x3& m);

        static const quaternion identity;
    };

    struct rot3x3
    {
        vec3 xBasis, yBasis, zBasis;

        /**
         * Default constructor that produces a matrix with all zeroes. A matrix with all zeroes is invalid by definition. Only use this
         * constructor when directly assigning the values before use.
         */
        constexpr rot3x3() noexcept
            : xBasis(0, 0, 0),
              yBasis(0, 0, 0),
              zBasis(0, 0, 0) { }

        /**
         * Construct a rotation matrix from basis vectors.
         * The vectors are assumed to be normal vectors and orthogonal to each other;
         * using this constructor with non-orthonormal basis vectors will result in an invalid rotation matrix.
         */
        constexpr rot3x3(vec3 x, vec3 y, vec3 z) noexcept : xBasis(x), yBasis(y), zBasis(z) { }

        /**
         * Construct a matrix from individual values. The columns of the matrix represent the rotation basis vectors.
         * Values are assumed to be orthonormal; using this constructor with non-orthonormal values will result in an invalid rotation matrix.
         * Argument name format: m#row#col (ex: m21 is the value in the third row, second column)
         */
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
        static rot3x3 eulerAngles(float x, float y, float z);
        static rot3x3 eulerAngles(vec3 angles);
        static rot3x3 lookRotation(const vec3& forward, const vec3& up);
        static rot3x3 angleAxis(float angle, const vec3& axis);

        static const rot3x3 identity;
    };

    inline const quaternion quaternion::identity = quaternion(0, 0, 0, 1);

    quaternion operator*(const quaternion& lhs, const quaternion& rhs) noexcept;
    constexpr bool approx(const quaternion& lhs, const quaternion& rhs, float eps = EPSILON) noexcept;

    constexpr quaternion inverse(quaternion q);
    inline quaternion normalize(quaternion q);
    constexpr float dot(quaternion a, quaternion b);
    inline vec3 toEuler(quaternion q);
    inline quaternion slerp(quaternion a, quaternion b, float t);
    inline vec3 rotate(const quaternion& q, const vec3& v);

    inline vec3 toEuler(const rot3x3& rot);

    constexpr bool quaternion::operator==(const quaternion& other) const
    {
        return other.x == x && other.y == y && other.z == z && other.w == w;
    }

    constexpr bool quaternion::operator!=(const quaternion& other) const
    {
        return !this->operator==(other);
    }

    constexpr quaternion inverse(quaternion q)
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

        const float sx = sin(x * 0.5f);
        const float cx = cos(x * 0.5f);
        const float sy = sin(y * 0.5f);
        const float cy = cos(y * 0.5f);
        const float sz = sin(z * 0.5f);
        const float cz = cos(z * 0.5f);

        quaternion q;
        q.w = cx * cy * cz + sx * sy * sz;
        q.x = sx * cy * cz - cx * sy * sz;
        q.y = cx * sy * cz + sx * cy * sz;
        q.z = cx * cy * sz - sx * sy * cz;
        return normalize(q);
    }

    inline quaternion quaternion::eulerAngles(vec3 angles)
    {
        return eulerAngles(angles.x, angles.y, angles.z);
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
        // Adapted from https://gamemath.com/book/orient.html#matrix_to_quaternion (listing 8.5), originally Shoemake
        float m11 = m.xBasis.x, m12 = m.yBasis.x, m13 = m.zBasis.x;
        float m21 = m.xBasis.y, m22 = m.yBasis.y, m23 = m.zBasis.y;
        float m31 = m.xBasis.z, m32 = m.yBasis.z, m33 = m.zBasis.z;

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
    // FREE FUNCTIONS    // ****

    inline quaternion operator*(const quaternion& lhs, const quaternion& rhs) noexcept
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
        assert(square > EPSILON); // Panic from zero-quaternion (never allowed)
        const float inv = 1.f / sqrt(square);
        return quaternion(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
    }

    constexpr bool approx(const quaternion& lhs, const quaternion& rhs, float eps) noexcept
    {
        return math::approx(lhs.x, rhs.x, eps)
               && math::approx(lhs.y, rhs.y, eps)
               && math::approx(lhs.z, rhs.z, eps)
               && math::approx(lhs.w, rhs.w, eps);
    }

    constexpr float dot(quaternion a, quaternion b)
    {
        return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline vec3 toEuler(quaternion q)
    {
        float sinp = -2 * (q.x * q.z - q.w * q.y);
        sinp = clamp(sinp, -1.f, 1.f);

        vec3 euler;
        if (abs(sinp) > 0.999999f)
        {
            euler.x = atan2(-2.0f * (q.y*q.z - q.w*q.x),
                            1.0f - 2.0f * (q.x * q.x + q.z * q.z));
            euler.y = HALFPI * sinp;
            euler.z = 0;
        }
        else
        {
            euler.x = atan2(2.0f * (q.y*q.z + q.w*q.x),
                            1.0f - 2.0f * (q.x*q.x + q.y*q.y));
            euler.y = asin(sinp);
            euler.z = atan2(2.0f * (q.x*q.y + q.w*q.z),
                            1.0f - 2.0f * (q.y * q.y + q.z * q.z));
        }

        return euler * RAD2DEG;
    }

    // inline vec3 toEuler(quaternion q)
    // {
    //     rot3x3 rot = rot3x3::fromQuaternion(q);
    //     return toEuler(rot);
    // }


    inline quaternion slerp(quaternion a, quaternion b, float t)
    {
        return quaternion::identity;
    }

    inline vec3 rotate(const quaternion& q, const vec3& v)
    {
        vec3 qv(q.x, q.y, q.z);
        vec3 uv = cross(qv, v);
        vec3 uuv = cross(qv, uv);
        return v + ((uv * q.w) + uuv) * 2.f;
    }

    inline const rot3x3 rot3x3::identity = rot3x3{
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

    constexpr rot3x3 rot3x3::fromQuaternion(quaternion q)
    {
        const float xx = q.x * q.x;
        const float yy = q.y * q.y;
        const float zz = q.z * q.z;
        const float xy = q.x * q.y;
        const float xz = q.x * q.z;
        const float yz = q.y * q.z;
        const float wx = q.w * q.x;
        const float wy = q.w * q.y;
        const float wz = q.w * q.z;

        rot3x3 m;
        m.xBasis = vec3(1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy));
        m.yBasis = vec3(2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx));
        m.zBasis = vec3(2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy));
        return m;
    }

    inline rot3x3 rot3x3::eulerAngles(float x, float y, float z)
    {
        x *= DEG2RAD;
        y *= DEG2RAD;
        z *= DEG2RAD;

        const float sx = sin(x), cx = cos(x);
        const float sy = sin(y), cy = cos(y);
        const float sz = sin(z), cz = cos(z);

        // R = Rz * Ry * Rx
        rot3x3 m;
        m.xBasis = vec3(cz * cy, sz * cy, -sy);
        m.yBasis = vec3(cz * sy * sx - sz * cx, sx * sy * sz + cz * cx, cy * sx);
        m.zBasis = vec3(cz * sy * cx + sz * sx, sz * sy * cx - cz * sx, cy * cx);
        return m;
    }

    inline rot3x3 rot3x3::eulerAngles(vec3 angles)
    {
        return eulerAngles(angles.x, angles.y, angles.z);
    }

    inline rot3x3 rot3x3::lookRotation(const vec3& forward, const vec3& up)
    {
        vec3 f = normalize(forward); // local y
        vec3 r = normalize(cross(f, up)); // local x
        vec3 u = cross(r, f); // local z
        return rot3x3(r, f, u);
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

        const float x = norm.x;
        const float y = norm.y;
        const float z = norm.z;
        const float t = 1.0 - c;

        rot3x3 m;
        m.xBasis = vec3(t * x * x + c, t * x * y - s * z, t * x * z + s * y);
        m.yBasis = vec3(t * x * y + s * z, t * y * y + c, t * y * z - s * x);
        m.zBasis = vec3(t * x * z - s * y, t * y * z + s * x, t * z * z + c);
        return m;
    }

    inline rot3x3 orthonormalize(const rot3x3& in)
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

    inline vec3 toEuler(const rot3x3& m)
    {
        float x, y, z;

        // R = Rz * Ry * Rx
        float sy = -m.xBasis.z; // -sinY
        if (sy <= -1.f)
        {
            y = -HALFPI;
        }
        else if (sy >= 1.f)
        {
            y = HALFPI;
        }
        else
        {
            y = asin(sy);
        }

        if (abs(sy) < .999999f)
        {
            x = atan2(m.yBasis.z, m.zBasis.z);
            z = atan2(m.xBasis.y, m.xBasis.x);
        }
        else
        {
            // gimbal lock
            x = atan2(-m.zBasis.y, m.yBasis.y);
            z = 0.0f;
        }
        return vec3(x, y, z) * RAD2DEG;
    }
}
