#pragma once
#include "math/vec3.h"

namespace math {
    struct AABB;
}

namespace core
{
    struct SphereCollider
    {
        float radius = 1;
        math::vec3 offset = math::vec3::zero;

        static SphereCollider fromAABB(const math::AABB& bounds);
    };

    struct BoxCollider
    {
        math::vec3 center = math::vec3::zero;
        math::vec3 extent = math::vec3::one;

        static BoxCollider fromAABB(const math::AABB& bounds);
    };

    struct CapsuleCollider
    {
        float height = 0;
        float radius = 1;
        math::vec3 offset  = math::vec3::zero;

        static CapsuleCollider fromAABB(const math::AABB& bounds);
    };
}
