#include "core/Transform.h"

#include <cassert>
#include <spdlog/spdlog.h>

using namespace math;
using namespace core;

namespace
{
    mat4 localTrsFromTransform(const Transform& t)
    {
        return mat4::makeTRS(t.localPosition, t.localRotation, t.localScale);
    }
}

TransformHandle TransformSystem::addTransform(flecs::entity e, TransformHandle parent)
{
    assert(isAlive(parent));

    const TransformHandle handle = claimHandle();

    transforms[handle.id] = Transform();
    worldNodes[handle.id] = WorldNode{parent.id, worldNodes[parent.id].worldMatrix, 0};

    e.set<TransformHandle>(handle);
    return handle;
}

TransformHandle TransformSystem::addTransform(flecs::entity e, TransformHandle parent, vec3 pos, quaternion rot, bool worldSpace)
{
    assert(isAlive(parent)); // todo replace c assert

    const TransformHandle handle = claimHandle();

    const mat4 localTRS = mat4::makeTRS(pos, rot, vec3::one);

    if (worldSpace)
    {
        // TODO [transform = local * invParent] but then extract the values back into local??
    }
    else
    {
        transforms[handle.id] = Transform{pos, rot, vec3::one};
        worldNodes[handle.id] = WorldNode{parent.id, worldNodes[parent.id].worldMatrix * localTRS, 0};
    }

    e.set<TransformHandle>(handle);
    return handle;
}

TransformHandle TransformSystem::addTransform(flecs::entity e, vec3 pos, quaternion rot)
{
    const TransformHandle handle = claimHandle();

    transforms[handle.id] = Transform{pos, rot, vec3::one};
    worldNodes[handle.id] = WorldNode{0, mat4::makeTRS(pos, rot, vec3::one), 0};

    e.set<TransformHandle>(handle);
    return handle;
}

TransformHandle TransformSystem::addTransform(flecs::entity e)
{
    const TransformHandle handle = claimHandle();

    transforms[handle.id] = Transform{ };
    worldNodes[handle.id] = WorldNode{ };

    e.set<TransformHandle>(handle);
    return handle;
}

bool TransformSystem::isAlive(TransformHandle handle) const
{
    return handle.id > 0 && handle.id < globalHead && worldNodes[handle.id].gen == handle.gen;
}

void TransformSystem::releaseHandle(TransformHandle handle)
{
    assert(isAlive(handle)); // TODO replace c assert

    worldNodes[handle.id].gen++;
    worldNodes[handle.id].parent = freeListHead;
    freeListHead = handle.id;

    // TODO handle children that point to this parent (implement strategy in a public api function, this place is more low-level)

    count--;
    freeListCount++;
}

TransformSystem::TransformSystem(flecs::world* world)
    : world(world)
{
    // Zero-initialize first element.
    // Note that it uses an absolute zero matrix instead of identity to cement its invalidity.
    transforms[0] = { };
    worldNodes[0] = {0, mat4::zero, 0};
    globalHead = 1;
}

TransformHandle TransformSystem::claimHandle()
{
    TransformIndex next;
    if (freeListHead > 0)
    {
        next = freeListHead;
        freeListHead = worldNodes[freeListHead].parent;

        freeListCount--;
    }
    else
    {
        next = globalHead;
        worldNodes[next].gen = 0;
        globalHead++;

        // TODO vector implementation needs to be able to grow, should be removed once vector is replaced
        if (globalHead >= worldNodes.size())
        {
            size_t newSize = worldNodes.size() + 50'000;
            transforms.resize(newSize);
            worldNodes.resize(newSize);
            spdlog::warn("Reallocating transform memory! New size is {}.", newSize);
        }
    }
    count++;

    return {next, worldNodes[next].gen};
}

void TransformSystem::setParent(TransformHandle handle, TransformHandle parent, bool keepWorldTransform)
{
    assert(isAlive(handle));
    assert(isAlive(parent));

    if (keepWorldTransform)
    {
        // Change local transform to match the current world position based on new parent's transform, world matrix doesn't change
        // TODO
    }
    else
    {
        // Local transform remains unchanged, so reinterpret it based on the new parent transform
        worldNodes[handle.id].parent = parent.id;
        worldNodes[handle.id].worldMatrix = worldNodes[parent.id].worldMatrix * localTrsFromTransform(transforms[handle.id]);
    }
}

TransformHandle TransformSystem::getParent(TransformHandle handle) const
{
    assert(isAlive(handle));

    // Note: if parent is zero, this will index nodes[0] which is an invalid node by definition, defined with gen = 0.
    // So this implicitly returns handle{0,0} aka handle::invalid, which is a conscious safety-performance tradeoff.
    const TransformIndex parent = worldNodes[handle.id].parent;
    return {parent, worldNodes[parent].gen};
}

bool TransformSystem::hasParent(TransformHandle handle) const
{
    assert(isAlive(handle));

    return worldNodes[handle.id].parent > 0;
}

void TransformSystem::setLocalPosition(TransformHandle handle, vec3 pos)
{
    assert(isAlive(handle));

    transforms[handle.id].localPosition = pos;
    // TODO modify/dirty
}

void TransformSystem::setLocalRotation(TransformHandle handle, quaternion rot)
{
    assert(isAlive(handle));
    transforms[handle.id].localRotation = rot;
    // TODO modify/dirty
}

void TransformSystem::setLocalScale(TransformHandle handle, vec3 scale)
{
    assert(isAlive(handle));
    transforms[handle.id].localScale = scale;
    // Todo modify/dirty
}

void TransformSystem::setLocalPosRot(TransformHandle handle, vec3 pos, quaternion rot)
{
    assert(isAlive(handle));
    transforms[handle.id].localPosition = pos;
    transforms[handle.id].localRotation = rot;
    // Todo modify/dirty
}

void TransformSystem::setWorldPosition(TransformHandle handle, vec3 pos)
{
    assert(isAlive(handle));
    // todo
}

void TransformSystem::setWorldRotation(TransformHandle handle, quaternion rot)
{
    assert(isAlive(handle));
    //todo
}

void TransformSystem::translate(TransformHandle handle, vec3 offset)
{
    assert(isAlive(handle));

    transforms[handle.id].localPosition = transforms[handle.id].localPosition + offset;
    // TODO dirty
}

void TransformSystem::rotate(TransformHandle handle, quaternion rot)
{
    assert(isAlive(handle));

    transforms[handle.id].localRotation = transforms[handle.id].localRotation * rot;
    // TODO dirty
}

vec3 TransformSystem::getLocalPosition(TransformHandle handle) const
{
    assert(isAlive(handle));
    return transforms[handle.id].localPosition;
}

quaternion TransformSystem::getLocalRotation(TransformHandle handle) const
{
    assert(isAlive(handle));
    return transforms[handle.id].localRotation;
}

vec3 TransformSystem::getLocalScale(TransformHandle handle) const
{
    assert(isAlive(handle));
    return transforms[handle.id].localScale;
}

vec3 TransformSystem::getWorldPosition(TransformHandle handle) const
{
    assert(isAlive(handle));
    //todo
    return vec3::zero;
}

quaternion TransformSystem::getWorldRotation(TransformHandle handle) const
{
    assert(isAlive(handle));
    //todo
    return quaternion::identity;
}

math::mat4 TransformSystem::getWorldMatrix(TransformHandle handle) const
{
    assert(isAlive(handle));

    return worldNodes[handle.id].worldMatrix;
}

vec3 TransformSystem::getRight(TransformHandle handle) const
{
    assert(isAlive(handle));
    //todo
    return vec3::zero;
}

vec3 TransformSystem::getUp(TransformHandle handle) const
{
    assert(isAlive(handle));
    //todo
    return vec3::zero;
}

vec3 TransformSystem::getForward(TransformHandle handle) const
{
    assert(isAlive(handle));
    //todo
    return vec3::zero;
}
