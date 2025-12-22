#pragma once

#include <flecs.h>
#include <vector>

#include "math/mat4.h"
#include "math/rotation.h"
#include "math/vec3.h"

namespace core
{
    typedef uint32_t TransformIndex;

    struct TransformHandle
    {
        TransformIndex id;
        uint32_t gen;

        static const TransformHandle nullHandle;
    };

    inline const TransformHandle TransformHandle::nullHandle = {0, 0};
}

namespace core
{
    struct Transform
    {
        math::vec3 localPosition = math::vec3::zero;
        math::quaternion localRotation = math::quaternion::identity;
        math::vec3 localScale = math::vec3::one;
        bool dirty = false;
    };

    struct WorldNode
    {
        TransformIndex parent = 0;

        math::mat4 worldMatrix = math::mat4::identity;
        uint32_t gen = 0;
    };

    struct TransformSystem
    {
    private:
        flecs::world* world;

        // TODO replace storage solution (paged storage? custom vector allocator? Memory arena?)
        //  paged storage is pointer-stable which could introduce some nice performance benefit (less indirection)
        //  Child pages are also an interesting idea: Larger pages 4k-32k entity pages for root objects and then smaller
        //  pages pages for children. Either smaller pages for direct children at every level, or larger pages for all
        //  the children at every level under a single root.
        std::vector<Transform> transforms = std::vector<Transform>(100'000);
        std::vector<WorldNode> worldNodes = std::vector<WorldNode>(100'000);

        TransformIndex globalHead = 1;
        TransformIndex freeListHead = 0;

        uint32_t count = 0;
        uint32_t freeListCount = 0;

        // Id handle management
        TransformHandle claimHandle();
        void releaseHandle(TransformHandle handle);

    public:
        explicit TransformSystem(flecs::world* world);

        //## Creation/destruction ##//

        /**
         * Creates an unparented transform on an entity at the world origin.
         * This entity is created with a TransformData component holding the transform index.
         */
        TransformHandle addTransform(flecs::entity e);

        /**
         * Creates an unparented transform on an entity with TRS values.
         * This entity is created with a TransformData component holding the transform index.
         */
        TransformHandle addTransform(flecs::entity e, math::vec3 pos, math::quaternion rot);

        /**
         * Create a parented entity in the world, with a 'zero' transform relative to the parent (ie in the same
         * location as the parent).
         * This entity is created with a TransformData component holding the transform index.
         */
        TransformHandle addTransform(flecs::entity e, TransformHandle parent);

        /**
         * Creates a parented transform on an entity. When the worldSpace parameter is set to true, the transform values will
         * be interpreted in world space (ie relative to the world origin), else (default) they will be interpreted in
         * local space (ie relative to the parent).
         * This entity is created with a TransformData component holding the transform index.
         */
        TransformHandle addTransform(flecs::entity e, TransformHandle parent, math::vec3 pos, math::quaternion rot, bool worldSpace = false);

        bool isAlive(TransformHandle handle) const;

        //## Hierarchy ##//
        void setParent(TransformHandle handle, TransformHandle parent, bool keepWorldTransform = true);
        TransformHandle getParent(TransformHandle handle) const;
        bool hasParent(TransformHandle handle) const;

        //## Spatial value getters/setters ##//
        void setLocalPosition(TransformHandle handle, math::vec3 pos);
        void setLocalRotation(TransformHandle handle, math::quaternion rot);
        void setLocalScale(TransformHandle handle, math::vec3 scale);
        void setLocalPosRot(TransformHandle handle, math::vec3 pos, math::quaternion rot);

        void setWorldPosition(TransformHandle handle, math::vec3 pos);
        void setWorldRotation(TransformHandle handle, math::quaternion rot);

        void translate(TransformHandle handle, math::vec3 offset);
        void rotate(TransformHandle handle, math::quaternion rot);

        math::vec3 getLocalPosition(TransformHandle handle) const;
        math::quaternion getLocalRotation(TransformHandle handle) const;
        math::vec3 getLocalScale(TransformHandle handle) const;

        math::vec3 getWorldPosition(TransformHandle handle) const;
        math::quaternion getWorldRotation(TransformHandle handle) const;

        math::mat4 getWorldMatrix(TransformHandle handle) const;

        math::vec3 getRight(TransformHandle handle) const;
        math::vec3 getUp(TransformHandle handle) const;
        math::vec3 getForward(TransformHandle handle) const;
    };
}
