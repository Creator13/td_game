#include "Collider.h"

#include "assets/Mesh.h"

using namespace core;
using namespace math;

SphereCollider SphereCollider::fromAABB(const AABB& bounds)
{
    return SphereCollider{
        .radius = max(bounds.halfExtents),
        .offset = bounds.center
    };
}

BoxCollider BoxCollider::fromAABB(const AABB& bounds)
{
    return BoxCollider{
        .center = bounds.center,
        .extent = bounds.halfExtents
    };
}

CapsuleCollider CapsuleCollider::fromAABB(const AABB& bounds)
{
    const float radius = max(bounds.halfExtents.xy());

    return CapsuleCollider{
        .height = max(EPSILON, bounds.halfExtents.z - radius * 2),
        .radius = radius,
        .offset = bounds.center,
    };
}
