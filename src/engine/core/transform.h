#pragma once

#include "math/mat4.h"
#include "math/rotation.h"
#include "math/vec3.h"

namespace core::ecs
{
struct TransformData;

struct WorldTransformData
{
    math::mat4 worldTransformMatrix;
    mutable bool propagateChange;
};

struct TransformData
{
private:
    math::vec3 localPosition = math::vec3::zero;
    math::quaternion localRotation = math::quaternion::identity;
    math::vec3 localScale = math::vec3::one;

    math::mat4 cachedLocalTRS;
    mutable bool dirty = true;

public:
    TransformData() = default;

    TransformData(math::vec3 pos, math::quaternion rot, math::vec3 scl)
        : localPosition(pos), localRotation(rot), localScale(scl) { }

    bool isDirty() const { return dirty; }
    void markClean() const { dirty = false; }

    // Local
    constexpr const math::vec3& getLocalPosition() const { return localPosition; }
    constexpr const math::quaternion& getLocalRotation() const { return localRotation; }
    constexpr const math::vec3& getLocalScale() const { return localScale; }

    void setLocal(const math::vec3& position, const math::quaternion& rotation, const math::vec3& scale);
    void setLocal(const math::vec3& position, const math::quaternion& rotation);

    void setLocalPosition(math::vec3 position);
    void setLocalRotation(math::quaternion rotation);
    void setLocalScale(math::vec3 scale);

    void translate(math::vec3 vec);
    void rotate(math::quaternion rot);
    void scale(math::vec3 scale);
    void scale(int scale);

    // TRS
    math::mat4 ensureTRS();
    math::mat4 getCachedTRS() const { return cachedLocalTRS; }
};
}
