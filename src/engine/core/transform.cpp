#include "core/transform.h"

using namespace math;
using namespace core::ecs;

mat4& WorldTransformData::getMatrix(TransformData* local, flecs::entity ett_this)
{
    flecs::entity parent = ett_this.parent();
    if (parent.is_alive())
    {

    }
}

void TransformData::setLocal(const vec3& position, const quaternion& rotation, const vec3& scale)
{
    localPosition = position;
    localRotation = rotation;
    localScale = scale;
    gen++;
}

void TransformData::setLocal(const vec3& position, const quaternion& rotation)
{
    localPosition = position;
    localRotation = rotation;
    gen++;
}

void TransformData::setLocalPosition(vec3 position)
{
    localPosition = position;
    gen++;
}

void TransformData::setLocalRotation(quaternion rotation)
{
    localRotation = rotation;
    gen++;
}

void TransformData::setLocalScale(vec3 scale)
{
    localScale = scale;
    gen++;
}

void TransformData::translate(vec3 vec)
{
    localPosition += vec;
    gen++;
}

void TransformData::rotate(quaternion rot)
{
    localRotation = localRotation * rot;
    gen++;
}

void TransformData::scale(vec3 scale)
{
    localScale = compMul(localScale, scale);
    gen++;
}

void TransformData::scale(int scale)
{
    localScale *= scale;
    gen++;
}

vec3 TransformData::getWorldPosition(flecs::entity ett_this, WorldTransformData* world_this)
{
    mat4& worldMatrix = world_this->getMatrix(this, ett_this);
    return extractPosition(worldMatrix);
}

quaternion TransformData::getWorldRotation(flecs::entity ett_this, WorldTransformData* world_this) { }

vec3 TransformData::getWorldScale(flecs::entity ett_this, WorldTransformData* world_this) { }

vec3 TransformData::getForward(flecs::entity ett_this, WorldTransformData* world_this)
{

}
vec3 TransformData::getRight(flecs::entity ett_this, WorldTransformData* world_this) { }
vec3 TransformData::getUp(flecs::entity ett_this, WorldTransformData* world_this) { }
void TransformData::setWorldPosition(vec3 position, flecs::entity ett_this, WorldTransformData* world_this) { }
void TransformData::setWorldRotation(quaternion rotation, flecs::entity ett_this, WorldTransformData* world_this) { }
