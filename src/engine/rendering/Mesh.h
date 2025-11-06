#pragma once

#include <vector>

#include "math/vec3.h"

namespace graphics
{
    struct Vertex
    {
        math::vec3 position;
        math::vec3 normal;
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };
}