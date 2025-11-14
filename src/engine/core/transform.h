#pragma once
#include "math/mat4.h"
#include "math/rotation.h"
#include "math/vec3.h"

namespace core
{
    struct TransformData
    {
        math::vec3 localPosition;
        math::vec3 localScale;
        math::quaternion localRotation;

        void translate(const math::vec3& vec);
        void scale(const math::vec3& vec);
        void rotate(const math::quaternion& rot);

        const math::mat4& getMatrix() const;

    private:
        mutable bool _dirty = true;
        mutable math::mat4 _matrix = math::mat4::identity;
    };
}
