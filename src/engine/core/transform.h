#pragma once

#include "math/mat4.h"
#include "math/rotation.h"
#include "math/vec3.h"

namespace core::ecs
{
    struct TransformData
    {
        math::vec3 localPosition;
        math::quaternion localRotation;
        math::vec3 localScale;

        void translate(const math::vec3& vec);
        void scale(const math::vec3& vec);
        void rotate(const math::quaternion& rot);
    };

    struct WorldTransformData
    {
        math::vec3 worldPosition;
        math::quaternion worldRotation;
        math::vec3 worldScale;

        math::mat4 worldTransformMatrix;
    };

    struct Dirty { };
}
