#pragma once
#include "math/vec.h"

namespace core
{
    struct Vertex
    {
        math::vec3 position;
        math::vec3 normal;
        math::vec2 uv0;
        math::vec4 tangent;
    };
}
