#include "core/transform.h"

using namespace math;
using namespace core::ecs;

void TransformData::setLocal(const vec3& position, const quaternion& rotation, const vec3& scale)
{
    localPosition = position;
    localRotation = rotation;
    localScale = scale;
    dirty = true;
}

void TransformData::setLocal(const vec3& position, const quaternion& rotation)
{
    localPosition = position;
    localRotation = rotation;
    dirty = true;
}

void TransformData::setLocalPosition(vec3 position)
{
    localPosition = position;
    dirty = true;
}

void TransformData::setLocalRotation(quaternion rotation)
{
    localRotation = rotation;
    dirty = true;
}

void TransformData::setLocalScale(vec3 scale)
{
    localScale = scale;
    dirty = true;
}

void TransformData::translate(vec3 vec)
{
    localPosition += vec;
    dirty = true;
}

void TransformData::rotate(quaternion rot)
{
    localRotation = localRotation * rot;
    dirty = true;
}

void TransformData::scale(vec3 scale)
{
    localScale = compMul(localScale, scale);
    dirty = true;
}

void TransformData::scale(int scale)
{
    localScale *= scale;
    dirty = true;
}

mat4 TransformData::ensureTRS()
{
    cachedLocalTRS = mat4::makeTRS(localPosition, localRotation, localScale);
    return cachedLocalTRS;
}
