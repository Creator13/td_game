#pragma once
#include "vec3.h"

namespace math
{
    struct AABB
    {
        vec3 min;
        vec3 max;
    };

    struct Sphere
    {
        vec3 center;
        float radius;
    };

    struct Circle
    {
        vec3 center;
        float radius;
    };

    struct Plane
    {
        vec3 normal;
        float dist;
    };

    struct Line
    {
        vec3 start;
        vec3 end;
    };

    struct Ray
    {
        vec3 start;
        vec3 dir;
    };

    struct Frustum
    {
        Plane top;
        Plane bottom;

        Plane right;
        Plane left;

        Plane far;
        Plane near;
    };
}
