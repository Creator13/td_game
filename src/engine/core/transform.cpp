#include "transform.h"

using namespace math;
using namespace core::ecs;

void TransformData::translate(const vec3& vec)
{
    localPosition += vec;
}

void TransformData::scale(const vec3& vec)
{
    localScale = compMul(localScale, vec);
}

void TransformData::rotate(const quaternion& rot)
{
    localRotation = localRotation * rot;
}
