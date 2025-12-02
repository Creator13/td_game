#pragma once

#include <flecs.h>

#include "math/mat4.h"
#include "math/rotation.h"
#include "math/vec3.h"

namespace core::ecs
{
struct TransformData;

struct WorldTransformData
{
    math::mat4 worldTransformMatrix;

    uint32_t cachedLocalGen = 0; // Version number of the TransformData.gen used to calculate the currently cached matrix

    uint32_t worldGen = 0; // Version number of the world matrix, independent of local gen
    uint32_t cachedParentWorldGen = 0; // Version number of the parent used to calculate the currently cached matrix
    flecs::entity_t cachedParentId; // ID of the parent used to calculate the currently cached matrix

    bool isDirty() const;
    math::mat4& getMatrix(TransformData* local, flecs::entity ett_this);
};

struct TransformData
{
private:
    math::vec3 localPosition = math::vec3::zero;
    math::quaternion localRotation = math::quaternion::identity;
    math::vec3 localScale = math::vec3::one;
    uint32_t gen = 1;

public:
    uint32_t getGen() const { return gen; }

    // Local
    const math::vec3& getLocalPosition() const { return localPosition; }
    const math::quaternion& getLocalRotation() const { return localRotation; }
    const math::vec3& getLocalScale() const { return localScale; }

    void setLocal(const math::vec3& position, const math::quaternion& rotation, const math::vec3& scale);
    void setLocal(const math::vec3& position, const math::quaternion& rotation);

    void setLocalPosition(math::vec3 position);
    void setLocalRotation(math::quaternion rotation);
    void setLocalScale(math::vec3 scale);

    void translate(math::vec3 vec);
    void rotate(math::quaternion rot);
    void scale(math::vec3 scale);
    void scale(int scale);

    // World
    math::vec3 getWorldPosition(flecs::entity ett_this, WorldTransformData* world_this);
    math::quaternion getWorldRotation(flecs::entity ett_this, WorldTransformData* world_this);
    math::vec3 getWorldScale(flecs::entity ett_this, WorldTransformData* world_this);

    math::vec3 getForward(flecs::entity ett_this, WorldTransformData* world_this);
    math::vec3 getRight(flecs::entity ett_this, WorldTransformData* world_this);
    math::vec3 getUp(flecs::entity ett_this, WorldTransformData* world_this);

    void setWorldPosition(math::vec3 position, flecs::entity ett_this, WorldTransformData* world_this);
    void setWorldRotation(math::quaternion rotation, flecs::entity ett_this, WorldTransformData* world_this);
};
}
