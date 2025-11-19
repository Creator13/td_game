#pragma once
#include <fastgltf/tools.hpp>

#include "math/vec.h"
#include "math/mat4.h"

namespace fastgltf
{
    template<>
    struct ElementTraits<::math::vec2> : ElementTraitsBase<::math::vec2, AccessorType::Vec2, float> {};

    template<>
    struct ElementTraits<::math::vec3> : ElementTraitsBase<::math::vec3, AccessorType::Vec3, float> {};

    template<>
    struct ElementTraits<::math::vec4> : ElementTraitsBase<::math::vec4, AccessorType::Vec4, float> {};

    template<>
    struct ElementTraits<::math::mat4> : ElementTraitsBase<::math::mat4, AccessorType::Mat4, float> {};
}
