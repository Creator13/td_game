#pragma once

#include <assert.h>
#include <span>

#include "rotation.h"
#include "vec3.h"
#include "vec4.h"

namespace math
{
    // Column-major representation
    struct mat4
    {
        float m[16];

        constexpr mat4() : m{ } { }

        // Format: m#row#col
        constexpr mat4(float m00, float m01, float m02, float m03, // ROW 0
                       float m10, float m11, float m12, float m13, // ROW 1
                       float m20, float m21, float m22, float m23, // ROW 2
                       float m30, float m31, float m32, float m33) // ROW 3
            : m{
                m00, m10, m20, m30, // COL 1
                m01, m11, m21, m31, // COL 2
                m02, m12, m22, m32, // COL 3
                m03, m13, m23, m33 // COL 4
            } { }

        /**
         * Constructor takes four 4D vectors for each of the columns of the matrix.
         */
        constexpr mat4(vec4 c0, vec4 c1, vec4 c2, vec4 c3)
            : m{
                c0.x, c0.y, c0.z, c0.w,
                c1.x, c1.y, c1.z, c1.w,
                c2.x, c2.y, c2.z, c2.w,
                c3.x, c3.y, c3.z, c3.w
            } { }

        // Operators: op*: const mat4, const mat4 -> mat4, op[]: i -> float, op==
        constexpr float operator[](int i) const { return m[i]; } // Raw indexer, column-major indexing
        constexpr bool operator==(const mat4& other) const;
        constexpr bool operator!=(const mat4& other) const;

        constexpr mat4& operator*=(const mat4& n);

        // Getters (const member functions): getCol: int -> float4, getRow: int -> float4, get: int, int -> float
        constexpr float get(int row, int col) const; // Explicit row-column indexer
        constexpr vec4 getRow(int row) const;
        constexpr vec4 getCol(int col) const;
        constexpr std::span<const float, 16> asSpan() { return {m}; };

        // Properties (as const member functions): getTranspose -> mat4, isIdentity -> bool, getInverse -> mat4, getDeterminant -> float
        constexpr mat4 getTranspose() const;
        constexpr bool isIdentity(float epsilon = EPSILON) const;
        constexpr float getDeterminant() const;
        constexpr mat4 getInverse() const;

        // Setters (mutating member functions): setCol: float4 -> void, setRow: float4 -> void, set: float -> void

        // Static factory functions: makeOrtho, makePerspective, makeLookAt, makeTranslate, makeRotate, makeScale, (makeTRS)
        static constexpr mat4 makeTranslate(vec3 t);
        static constexpr mat4 makeTranslate(float x, float y, float z);

        static constexpr mat4 makeScale(vec3 t);
        static constexpr mat4 makeScale(float x, float y, float z);

        constexpr static mat4 makeRotation(const rot3x3& rot);
        static mat4 makeRotation(quaternion rotation);

        constexpr static mat4 makeTRS(vec3 t, const rot3x3& r, vec3 s);
        static mat4 makeTRS(vec3 t, quaternion r, vec3 s);

        // default matrices: zero, identity
        static const mat4 zero, identity;
    };

    constexpr bool approx(const mat4& a, const mat4& b, float epsilon = EPSILON)
    {
        for (int i = 0; i < 16; ++i)
        {
            if (!approx(a.m[i], b.m[i], epsilon)) return false;
        }
        return true;
    }

    const mat4 mat4::zero = mat4();

    inline const mat4 mat4::identity = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    constexpr bool mat4::operator==(const mat4& other) const
    {
        for (int i = 0; i < 16; i++)
        {
            if (other[i] != m[i]) return false;
        }
        return true;
    }

    constexpr bool mat4::operator!=(const mat4& other) const
    {
        return !operator==(other);
    }

    constexpr mat4 operator*(const mat4& lhs, const mat4& rhs)
    {
        return mat4(
            lhs.m[0] * rhs.m[0] + lhs.m[4] * rhs.m[1] + lhs.m[8] * rhs.m[2] + lhs.m[12] * rhs.m[3],
            lhs.m[0] * rhs.m[4] + lhs.m[4] * rhs.m[5] + lhs.m[8] * rhs.m[6] + lhs.m[12] * rhs.m[7],
            lhs.m[0] * rhs.m[8] + lhs.m[4] * rhs.m[9] + lhs.m[8] * rhs.m[10] + lhs.m[12] * rhs.m[11],
            lhs.m[0] * rhs.m[12] + lhs.m[4] * rhs.m[13] + lhs.m[8] * rhs.m[14] + lhs.m[12] * rhs.m[15],

            lhs.m[1] * rhs.m[0] + lhs.m[5] * rhs.m[1] + lhs.m[9] * rhs.m[2] + lhs.m[13] * rhs.m[3],
            lhs.m[1] * rhs.m[4] + lhs.m[5] * rhs.m[5] + lhs.m[9] * rhs.m[6] + lhs.m[13] * rhs.m[7],
            lhs.m[1] * rhs.m[8] + lhs.m[5] * rhs.m[9] + lhs.m[9] * rhs.m[10] + lhs.m[13] * rhs.m[11],
            lhs.m[1] * rhs.m[12] + lhs.m[5] * rhs.m[13] + lhs.m[9] * rhs.m[14] + lhs.m[13] * rhs.m[15],

            lhs.m[2] * rhs.m[0] + lhs.m[6] * rhs.m[1] + lhs.m[10] * rhs.m[2] + lhs.m[14] * rhs.m[3],
            lhs.m[2] * rhs.m[4] + lhs.m[6] * rhs.m[5] + lhs.m[10] * rhs.m[6] + lhs.m[14] * rhs.m[7],
            lhs.m[2] * rhs.m[8] + lhs.m[6] * rhs.m[9] + lhs.m[10] * rhs.m[10] + lhs.m[14] * rhs.m[11],
            lhs.m[2] * rhs.m[12] + lhs.m[6] * rhs.m[13] + lhs.m[10] * rhs.m[14] + lhs.m[14] * rhs.m[15],

            lhs.m[3] * rhs.m[0] + lhs.m[7] * rhs.m[1] + lhs.m[11] * rhs.m[2] + lhs.m[15] * rhs.m[3],
            lhs.m[3] * rhs.m[4] + lhs.m[7] * rhs.m[5] + lhs.m[11] * rhs.m[6] + lhs.m[15] * rhs.m[7],
            lhs.m[3] * rhs.m[8] + lhs.m[7] * rhs.m[9] + lhs.m[11] * rhs.m[10] + lhs.m[15] * rhs.m[11],
            lhs.m[3] * rhs.m[12] + lhs.m[7] * rhs.m[13] + lhs.m[11] * rhs.m[14] + lhs.m[15] * rhs.m[15]
        );
    }

    constexpr vec4 operator*(const mat4& lhs, vec4 vec)
    {
        return vec4(
            dot(lhs.getRow(0), vec),
            dot(lhs.getRow(1), vec),
            dot(lhs.getRow(2), vec),
            dot(lhs.getRow(3), vec)
        );
    }

    constexpr mat4& mat4::operator*=(const mat4& n)
    {
        *this = *this * n;
        return *this;
    }

    constexpr float mat4::get(int row, int col) const
    {
        assert(col >= 0 && col < 4 && row >= 0 && row < 4);
        return m[col * 4 + row];
    }

    constexpr vec4 mat4::getRow(int row) const
    {
        assert(row >= 0 && row < 4);
        return vec4(
            m[row + 0 * 4],
            m[row + 1 * 4],
            m[row + 2 * 4],
            m[row + 3 * 4]);
    }

    constexpr vec4 mat4::getCol(int col) const
    {
        assert(col >= 0 && col < 4);
        const int base = col * 4;
        return vec4(
            m[base],
            m[base + 1],
            m[base + 2],
            m[base + 3]);
    }

    constexpr mat4 mat4::getTranspose() const
    {
        return mat4{
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]
        };
    }

    constexpr bool mat4::isIdentity(float epsilon) const
    {
        return
                approx(m[0], 1.0f, epsilon) &&
                approx(m[5], 1.0f, epsilon) &&
                approx(m[10], 1.0f, epsilon) &&
                approx(m[15], 1.0f, epsilon) &&
                approx(m[1], 0.0f, epsilon) &&
                approx(m[2], 0.0f, epsilon) &&
                approx(m[3], 0.0f, epsilon) &&
                approx(m[4], 0.0f, epsilon) &&
                approx(m[6], 0.0f, epsilon) &&
                approx(m[7], 0.0f, epsilon) &&
                approx(m[8], 0.0f, epsilon) &&
                approx(m[9], 0.0f, epsilon) &&
                approx(m[11], 0.0f, epsilon) &&
                approx(m[12], 0.0f, epsilon) &&
                approx(m[13], 0.0f, epsilon) &&
                approx(m[14], 0.0f, epsilon);
    }

    constexpr float mat4::getDeterminant() const
    {
        if (approx(m[3], 0) && approx(m[7], 0) && approx(m[11], 0) && approx(m[15], 0))
        {
            const float a = m[0], b = m[4], c = m[8];
            const float d = m[1], e = m[5], f = m[9];
            const float g = m[2], h = m[6], i = m[10];
            return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
        }
        else
        {
            const float a00 = m[0], a01 = m[4], a02 = m[8], a03 = m[12];
            const float a10 = m[1], a11 = m[5], a12 = m[9], a13 = m[13];
            const float a20 = m[2], a21 = m[6], a22 = m[10], a23 = m[14];
            const float a30 = m[3], a31 = m[7], a32 = m[11], a33 = m[15];

            return
                    a03 * a12 * a21 * a30 - a02 * a13 * a21 * a30 - a03 * a11 * a22 * a30 + a01 * a13 * a22 * a30 +
                    a02 * a11 * a23 * a30 - a01 * a12 * a23 * a30 - a03 * a12 * a20 * a31 + a02 * a13 * a20 * a31 +
                    a03 * a10 * a22 * a31 - a00 * a13 * a22 * a31 - a02 * a10 * a23 * a31 + a00 * a12 * a23 * a31 +
                    a03 * a11 * a20 * a32 - a01 * a13 * a20 * a32 - a03 * a10 * a21 * a32 + a00 * a13 * a21 * a32 +
                    a01 * a10 * a23 * a32 - a00 * a11 * a23 * a32 - a02 * a11 * a20 * a33 + a01 * a12 * a20 * a33 +
                    a02 * a10 * a21 * a33 - a00 * a12 * a21 * a33 - a01 * a10 * a22 * a33 + a00 * a11 * a22 * a33;
        }
    }

    constexpr mat4 mat4::getInverse() const
    {
        // Affine variant
        if (approx(m[3], 0) && approx(m[7], 0) && approx(m[11], 0) && approx(m[15], 0))
        {
            const float a = m[0], b = m[4], c = m[8];
            const float d = m[1], e = m[5], f = m[9];
            const float g = m[2], h = m[6], i = m[10];

            const float det3 = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
            if (approx(det3, 0)) return zero;

            const float invDet = 1.0f / det3;

            const float r00 = (e * i - f * h) * invDet;
            const float r01 = -(b * i - c * h) * invDet;
            const float r02 = (b * f - c * e) * invDet;

            const float r10 = -(d * i - f * g) * invDet;
            const float r11 = (a * i - c * g) * invDet;
            const float r12 = -(a * f - c * d) * invDet;

            const float r20 = (d * h - e * g) * invDet;
            const float r21 = -(a * h - b * g) * invDet;
            const float r22 = (a * e - b * d) * invDet;

            const float tx = m[12], ty = m[13], tz = m[14];
            const float invTx = -(r00 * tx + r01 * ty + r02 * tz);
            const float invTy = -(r10 * tx + r11 * ty + r12 * tz);
            const float invTz = -(r20 * tx + r21 * ty + r22 * tz);

            return mat4(
                r00, r01, r02, 0.0f,
                r10, r11, r12, 0.0f,
                r20, r21, r22, 0.0f,
                invTx, invTy, invTz, 1.0f
            );
        }
        // General inverse
        else
        {
            const float det = getDeterminant();
            if (approx(det, 0)) return zero;

            const float a00 = m[0], a01 = m[4], a02 = m[8], a03 = m[12];
            const float a10 = m[1], a11 = m[5], a12 = m[9], a13 = m[13];
            const float a20 = m[2], a21 = m[6], a22 = m[10], a23 = m[14];
            const float a30 = m[3], a31 = m[7], a32 = m[11], a33 = m[15];

            float inv[16];

            inv[0] = a11 * a22 * a33 - a11 * a23 * a32 - a21 * a12 * a33 + a21 * a13 * a32 + a31 * a12 * a23 - a31 * a13 * a22;
            inv[1] = -a10 * a22 * a33 + a10 * a23 * a32 + a20 * a12 * a33 - a20 * a13 * a32 - a30 * a12 * a23 + a30 * a13 * a22;
            inv[2] = a10 * a21 * a33 - a10 * a23 * a31 - a20 * a11 * a33 + a20 * a13 * a31 + a30 * a11 * a23 - a30 * a13 * a21;
            inv[3] = -a10 * a21 * a32 + a10 * a22 * a31 + a20 * a11 * a32 - a20 * a12 * a31 - a30 * a11 * a22 + a30 * a12 * a21;

            inv[4] = -a01 * a22 * a33 + a01 * a23 * a32 + a21 * a02 * a33 - a21 * a03 * a32 - a31 * a02 * a23 + a31 * a03 * a22;
            inv[5] = a00 * a22 * a33 - a00 * a23 * a32 - a20 * a02 * a33 + a20 * a03 * a32 + a30 * a02 * a23 - a30 * a03 * a22;
            inv[6] = -a00 * a21 * a33 + a00 * a23 * a31 + a20 * a01 * a33 - a20 * a03 * a31 - a30 * a01 * a23 + a30 * a03 * a21;
            inv[7] = a00 * a21 * a32 - a00 * a22 * a31 - a20 * a01 * a32 + a20 * a02 * a31 + a30 * a01 * a22 - a30 * a02 * a21;

            inv[8] = a01 * a12 * a33 - a01 * a13 * a32 - a11 * a02 * a33 + a11 * a03 * a32 + a31 * a02 * a13 - a31 * a03 * a12;
            inv[9] = -a00 * a12 * a33 + a00 * a13 * a32 + a10 * a02 * a33 - a10 * a03 * a32 - a30 * a02 * a13 + a30 * a03 * a12;
            inv[10] = a00 * a11 * a33 - a00 * a13 * a31 - a10 * a01 * a33 + a10 * a03 * a31 + a30 * a01 * a13 - a30 * a03 * a11;
            inv[11] = -a00 * a11 * a32 + a00 * a12 * a31 + a10 * a01 * a32 - a10 * a02 * a31 - a30 * a01 * a12 + a30 * a02 * a11;

            inv[12] = -a01 * a12 * a23 + a01 * a13 * a22 + a11 * a02 * a23 - a11 * a03 * a22 - a21 * a02 * a13 + a21 * a03 * a12;
            inv[13] = a00 * a12 * a23 - a00 * a13 * a22 - a10 * a02 * a23 + a10 * a03 * a22 + a20 * a02 * a13 - a20 * a03 * a12;
            inv[14] = -a00 * a11 * a23 + a00 * a13 * a21 + a10 * a01 * a23 - a10 * a03 * a21 - a20 * a01 * a13 + a20 * a03 * a11;
            inv[15] = a00 * a11 * a22 - a00 * a12 * a21 - a10 * a01 * a22 + a10 * a02 * a21 + a20 * a01 * a12 - a20 * a02 * a11;

            const float invDet = 1.0f / det;

            mat4 result;
            for (int i = 0; i < 16; ++i)
            {
                result.m[i] = inv[i] * invDet;
            }
            return result;
        }
    }

    constexpr mat4 mat4::makeTranslate(vec3 t)
    {
        return makeTranslate(t.x, t.y, t.z);
    }

    constexpr mat4 mat4::makeTranslate(float x, float y, float z)
    {
        return mat4(
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1);
    }

    constexpr mat4 mat4::makeScale(vec3 t)
    {
        return makeScale(t.x, t.y, t.z);
    }

    constexpr mat4 mat4::makeScale(float x, float y, float z)
    {
        return mat4(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1);
    }

    constexpr mat4 mat4::makeRotation(const rot3x3& rot)
    {
        return mat4(
            rot.xBasis.x, rot.yBasis.x, rot.zBasis.x, 0,
            rot.xBasis.y, rot.yBasis.y, rot.zBasis.y, 0,
            rot.xBasis.z, rot.yBasis.z, rot.zBasis.z, 0,
            0, 0, 0, 1
        );
    }

    inline mat4 mat4::makeRotation(quaternion rotation)
    {
        return makeRotation(rot3x3::fromQuaternion(rotation));
    }

    constexpr mat4 mat4::makeTRS(vec3 t, const rot3x3& r, vec3 s)
    {
        const vec4 xBasis = vec4(r.xBasis * s.x, 0);
        const vec4 yBasis = vec4(r.yBasis * s.y, 0);
        const vec4 zBasis = vec4(r.zBasis * s.z, 0);
        const vec4 w = vec4(t, 1);
        return mat4(xBasis, yBasis, zBasis, w);
    }

    inline mat4 mat4::makeTRS(vec3 t, quaternion r, vec3 s)
    {
        return makeTRS(t, rot3x3::fromQuaternion(r), s);
    }
}
