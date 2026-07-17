#include "core/Transform.h"

#include <tracy/Tracy.hpp>

#include "core/Assert.h"

using namespace core;
using namespace core::ecs;
using namespace math;

vec3 FreeTransform::getPosition() const
{
    return pos;
}

quaternion FreeTransform::getOrientation() const
{
    return rot;
}

vec3 FreeTransform::getScale() const
{
    return scale;
}

void FreeTransform::setPosition(vec3 newPos)
{
    this->pos = newPos;
}

void FreeTransform::setOrientation(quaternion newRot)
{
    this->rot = newRot;
    this->rotMatx = rot3x3::fromQuaternion(newRot);
}

void FreeTransform::setScale(vec3 scl)
{
    this->scale = scl;
}

void FreeTransform::rotate(quaternion rotation)
{
    rot = rot * rotation;
    this->rotMatx = rot3x3::fromQuaternion(rot);
}

void FreeTransform::translate(vec3 offset)
{
    pos += offset;
}

vec3 FreeTransform::getRight() const
{
    return rotMatx.xBasis;
}

vec3 FreeTransform::getUp() const
{
    return rotMatx.zBasis;
}

vec3 FreeTransform::getForward() const
{
    return rotMatx.yBasis;
}

mat4 FreeTransform::getWorldMatrix() const
{
    return mat4::makeTRS(pos, rotMatx, scale);
}

vec3 HierarchyTransform::getWorldPosition() const
{
    return worldMatrix.getCol(3).xyz();
}

quaternion HierarchyTransform::getGlobalOrientation() const
{
    vec3 col0 = worldMatrix.getCol(0).xyz();
    vec3 col1 = worldMatrix.getCol(1).xyz();
    vec3 col2 = worldMatrix.getCol(2).xyz();

    vec3 scale = vec3(
        col0.length(),
        col1.length(),
        col2.length()
    );

    if (scale.x > 0e-4f) col0 /= scale.x;
    if (scale.y > 0e-4f) col1 /= scale.y;
    if (scale.z > 0e-4f) col2 /= scale.z;

    const float det = dot(col0, cross(col1, col2));
    if (det < 0)
    {
        scale.x = -scale.x;
    }

    return quaternion::fromRot3x3(rot3x3(col0, col1, col2));
}

vec3 HierarchyTransform::getGlobalScale() const
{
    const vec3 col0 = worldMatrix.getCol(0).xyz();
    const vec3 col1 = worldMatrix.getCol(1).xyz();
    const vec3 col2 = worldMatrix.getCol(2).xyz();

    vec3 scale = vec3(
        col0.length(),
        col1.length(),
        col2.length()
    );

    const float det = dot(col0, cross(col1, col2));
    if (det < 0)
    {
        scale.x = -scale.x;
    }

    return scale;
}

vec3 HierarchyTransform::getLocalPosition() const
{
    return localPos;
}

quaternion HierarchyTransform::getLocalOrientation() const
{
    return localRot;
}

vec3 HierarchyTransform::getLocalScale() const
{
    return localScale;
}

void HierarchyTransform::setWorldPosition(flecs::entity e, vec3 pos)
{
    if (transform::hasParentTransform(e))
    {
        const mat4& parentMatx = e.parent().get<HierarchyTransform>().worldMatrix;
        localPos = (inverse(parentMatx) * vec4(pos, 1.0f)).xyz();
    }
    else
    {
        localPos = pos;
    }
    applyModified(e);
}

void HierarchyTransform::setGlobalOrientation(flecs::entity e, quaternion rot)
{
    if (transform::hasParentTransform(e))
    {
        const HierarchyTransform& parentTransform = e.parent().get<HierarchyTransform>();
        const quaternion parentWorldRot = parentTransform.getGlobalOrientation();
        localRot = inverse(parentWorldRot) * rot;
    }
    else
    {
        localRot = rot;
    }
    applyModified(e);
}

void HierarchyTransform::setLocalPosition(flecs::entity e, vec3 pos)
{
    localPos = pos;
    applyModified(e);
}

void HierarchyTransform::setLocalOrientation(flecs::entity e, quaternion rot)
{
    localRot = rot;
    applyModified(e);
}

void HierarchyTransform::setLocalScale(flecs::entity e, vec3 scl)
{
    localScale = scl;
    applyModified(e);
}

void HierarchyTransform::rotate(flecs::entity e, quaternion rotation)
{
    localRot = localRot * rotation;
    applyModified(e);
}

void HierarchyTransform::translate(flecs::entity e, vec3 offset)
{
    localPos += offset;
    applyModified(e);
}

vec3 HierarchyTransform::getRight() const
{
    return normalize(worldMatrix.getCol(0).xyz());
}

vec3 HierarchyTransform::getUp() const
{
    return normalize(worldMatrix.getCol(2).xyz());
}

vec3 HierarchyTransform::getForward() const
{
    return normalize(worldMatrix.getCol(1).xyz());
}

const mat4& HierarchyTransform::getWorldMatrix() const
{
    return worldMatrix;
}

void HierarchyTransform::applyModified(flecs::entity e_self)
{
    ZoneScoped;

    const mat4* parentMatx = nullptr;

    if (transform::hasParentEntity(e_self))
    {
        const HierarchyTransform* t_parent = e_self.parent().try_get<HierarchyTransform>();
        if (t_parent)
        {
            parentMatx = &(t_parent->worldMatrix);
        }
    }

    propagateMatrixToChildren(e_self, parentMatx);
}

void HierarchyTransform::propagateMatrixToChildren(flecs::entity e_self, const mat4* parentMatrix)
{
    if (parentMatrix != nullptr)
    {
        worldMatrix = *parentMatrix * mat4::makeTRS(localPos, localRot, localScale);
    }
    else
    {
        worldMatrix = mat4::makeTRS(localPos, localRot, localScale);
    }

    e_self.children([this](flecs::entity e_child)
    {
        HierarchyTransform* t_child = e_child.try_get_mut<HierarchyTransform>();
        t_child->propagateMatrixToChildren(e_child, &worldMatrix);
    });
}

void transform::add(flecs::entity target, flecs::entity parent, vec3 pos, quaternion rot, vec3 scale)
{
    ENGINE_ASSERT(!target.has<HierarchyTransform>(), "Tried to add transform to entity that already has a transform! Entity: {}", target.id());
    ENGINE_ASSERT(parent.is_alive(), "Transform parent is not a valid entity: parent id {}, target id {}", parent.id(), target.id());
    ENGINE_ASSERT(parent.has<HierarchyTransform>(), "Transform parent is not a transform itself: parent id {}, target id {}", parent.id(), target.id());

    target.child_of(parent);
    const mat4& parentMatx = parent.get<HierarchyTransform>().getWorldMatrix();
    const mat4 localMatx = mat4::makeTRS(pos, rot, scale);
    target.set<HierarchyTransform>({pos, rot, scale, parentMatx * localMatx});
}

void transform::add(flecs::entity target, flecs::entity parent, vec3 pos, quaternion rot)
{
    ENGINE_ASSERT(!target.has<HierarchyTransform>(), "Tried to add transform to entity that already has a transform! Entity: {}", target.id());
    ENGINE_ASSERT(parent.is_alive(), "Transform parent is not a valid entity: parent id {}, target id {}", parent.id(), target.id());
    ENGINE_ASSERT(parent.has<HierarchyTransform>(), "Transform parent is not a transform itself: parent id {}, target id {}", parent.id(), target.id());

    target.child_of(parent);
    const mat4& parentMatx = parent.get<HierarchyTransform>().getWorldMatrix();
    const mat4 localMatx = mat4::makeTRS(pos, rot, vec3::one);
    target.set<HierarchyTransform>({pos, rot, vec3::one, parentMatx * localMatx});
}

void transform::add(flecs::entity target, flecs::entity parent, vec3 pos)
{
    ENGINE_ASSERT(!target.has<HierarchyTransform>(), "Tried to add transform to entity that already has a transform! Entity: {}", target.id());
    ENGINE_ASSERT(parent.is_alive(), "Transform parent is not a valid entity: parent id {}, target id {}", parent.id(), target.id());
    ENGINE_ASSERT(parent.has<HierarchyTransform>(), "Transform parent is not a transform itself: parent id {}, target id {}", parent.id(), target.id());

    target.child_of(parent);
    const mat4& parentMatx = parent.get<HierarchyTransform>().getWorldMatrix();
    const mat4 localMatx = mat4::makeTRS(pos, quaternion::identity, vec3::one);
    target.set<HierarchyTransform>({pos, quaternion::identity, vec3::one, parentMatx * localMatx});
}

void transform::add(flecs::entity target, flecs::entity parent)
{
    ENGINE_ASSERT(!target.has<HierarchyTransform>(), "Tried to add transform to entity that already has a transform! Entity: {}", target.id());
    ENGINE_ASSERT(parent.is_alive(), "Transform parent is not a valid entity: parent id {}, target id {}", parent.id(), target.id());
    ENGINE_ASSERT(parent.has<HierarchyTransform>(), "Transform parent is not a transform itself: parent id {}, target id {}", parent.id(), target.id());

    target.set<HierarchyTransform>({ }); // Struct defaults to identity transform, so identity values are not explicitly set here
    target.child_of(parent);
}

void transform::add(flecs::entity target, vec3 pos, quaternion rot, vec3 scale)
{
    ENGINE_ASSERT(!target.has<HierarchyTransform>(), "Tried to add transform to entity that already has a transform! Entity: {}", target.id());

    const mat4 localMatx = mat4::makeTRS(pos, rot, scale);
    target.set<HierarchyTransform>({pos, rot, scale, localMatx});
}

void transform::add(flecs::entity target, vec3 pos, quaternion rot)
{
    ENGINE_ASSERT(!target.has<HierarchyTransform>(), "Tried to add transform to entity that already has a transform! Entity: {}", target.id());

    const mat4 localMatx = mat4::makeTRS(pos, rot, vec3::one);
    target.set<HierarchyTransform>({pos, rot, vec3::one, localMatx});
}

void transform::add(flecs::entity target, vec3 pos)
{
    ENGINE_ASSERT(!target.has<HierarchyTransform>(), "Tried to add transform to entity that already has a transform! Entity: {}", target.id());

    const mat4 localMatx = mat4::makeTRS(pos, quaternion::identity, vec3::one);
    target.set<HierarchyTransform>({pos, quaternion::identity, vec3::one, localMatx});
}

bool transform::hasParentEntity(flecs::entity e)
{
    return e.has(flecs::ChildOf, flecs::Wildcard);
}

bool transform::hasParentTransform(flecs::entity e)
{
    return hasParentEntity(e) && e.parent().has<HierarchyTransform>();
}
