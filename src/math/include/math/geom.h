#pragma once

#include "mat4.h"
#include "vec3.h"

namespace math
{
    struct AABB
    {
        vec3 center;
        vec3 halfExtents;

        constexpr static AABB fromMinMax(const vec3& min, const vec3& max)
        {
            const vec3 extents = max - min;
            const vec3 halfExtents = extents * .5f;
            const vec3 center = min + halfExtents;

            return AABB(center, halfExtents);
        }
    };

    struct rect
    {
        vec2 offset;
        vec2 extents;
    };

    constexpr rect translate(rect r, vec2 t)
    {
        r.offset += t;
        return r;
    }

    constexpr rect translate(rect r, float x, float y)
    {
        return translate(r, vec2(x, y));
    }

    constexpr rect translate(rect r, float c)
    {
        return translate(r, vec2(c, c));
    }

    constexpr rect scale(rect r, float s)
    {
        r.extents *= s;
        return r;
    }

    constexpr rect scale(rect r, vec2 s)
    {
        r.extents.x *= s.x;
        r.extents.y *= s.y;
        return r;
    }

    struct sphere
    {
        vec3 center;
        float radius;
    };

    struct circle
    {
        vec3 center;
        float radius;
    };

    struct plane
    {
        vec3 normal;
        float dist;

        MATH_CONSTEXPR_CMATH inline static plane fromMatrixRow(vec4 v)
        {
            const float len = v.xyz().length();
            return plane{
                vec3(v.x / len, v.y / len, v.z / len),
                v.w / len
            };
        }
    };

    constexpr float signedDistanceToPlane(const plane& plane, const vec3& point)
    {
        return dot(plane.normal, point) - plane.dist;
    }

    struct line
    {
        vec3 start;
        vec3 end;
    };

    struct ray
    {
        vec3 start;
        vec3 dir;
    };

    struct frustum
    {
        plane top;
        plane bottom;

        plane right;
        plane left;

        plane far;
        plane near;

        static frustum fromViewProjectionMatrix(const mat4& vpMat)
        {
            // TODO if gAPI ever changes, look at different Gribb-Hartmann method
            const vec4 row0 = vpMat.getRow(0);
            const vec4 row1 = vpMat.getRow(1);
            const vec4 row2 = vpMat.getRow(2);
            const vec4 row3 = vpMat.getRow(3);

            frustum f;
            f.left = plane::fromMatrixRow(row3 + row0);
            f.right = plane::fromMatrixRow(row3 - row0);
            f.bottom = plane::fromMatrixRow(row3 + row1);
            f.top = plane::fromMatrixRow(row3 - row1);
            f.near = plane::fromMatrixRow(row3 + row2);
            f.far = plane::fromMatrixRow(row3 - row2);
            return f;
        }
    };
}
