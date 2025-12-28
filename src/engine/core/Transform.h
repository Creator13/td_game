#pragma once

#include <flecs.h>

#include "math/mat4.h"
#include "math/rotation.h"
#include "math/vec3.h"

namespace core
{
    struct FreeTransform
    {
        math::vec3 pos;
        math::quaternion rot;
        math::vec3 scale;

        math::rot3x3 rotMatx;

        math::vec3 getPosition() const;
        math::quaternion getOrientation() const;
        math::vec3 getScale() const;

        void setPosition(math::vec3 newPos);
        void setOrientation(math::quaternion newRot);
        void setScale(math::vec3 scl);

        void rotate(math::quaternion rotation);
        void translate(math::vec3 offset);

        math::vec3 getRight() const;
        math::vec3 getUp() const;
        math::vec3 getForward() const;

        math::mat4 getWorldMatrix() const;
    };

    struct HierarchyTransform
    {
        math::vec3 localPos = math::vec3::zero;
        math::quaternion localRot = math::quaternion::identity;
        math::vec3 localScale = math::vec3::one;

        math::mat4 worldMatrix = math::mat4::identity;

        math::vec3 getWorldPosition() const;
        math::quaternion getGlobalOrientation() const;
        math::vec3 getGlobalScale() const;

        math::vec3 getLocalPosition() const;
        math::quaternion getLocalOrientation() const;
        math::vec3 getLocalScale() const;

        void setWorldPosition(flecs::entity e, math::vec3 pos);
        void setGlobalOrientation(flecs::entity e, math::quaternion rot);

        void setLocalPosition(flecs::entity e, math::vec3 pos);
        void setLocalOrientation(flecs::entity e, math::quaternion rot);
        void setLocalScale(flecs::entity e, math::vec3 scl);

        void rotate(flecs::entity e, math::quaternion rotation);
        void translate(flecs::entity e, math::vec3 offset);

        math::vec3 getRight() const;
        math::vec3 getUp() const;
        math::vec3 getForward() const;

        const math::mat4& getWorldMatrix() const;

        bool hasParentEntity(flecs::entity e);
        bool hasParentTransform(flecs::entity e);

    private:
        void applyModified(flecs::entity e_self);
        void propagateMatrixToChildren(flecs::entity e_self, const math::mat4* parentMatrix);
    };

    namespace transform
    {
        void add(flecs::entity target, flecs::entity parent, math::vec3 pos, math::quaternion rot, math::vec3 scale);
        void add(flecs::entity target, flecs::entity parent, math::vec3 pos, math::quaternion rot);
        void add(flecs::entity target, flecs::entity parent, math::vec3 pos);
        void add(flecs::entity target, flecs::entity parent);
        void add(flecs::entity target, math::vec3 pos, math::quaternion rot, math::vec3 scale);
        void add(flecs::entity target, math::vec3 pos, math::quaternion rot);
        void add(flecs::entity target, math::vec3 pos);
    }
}
