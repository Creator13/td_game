#pragma once

#include <cstdint>
#include <span>
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

    struct MeshView
    {
        std::span<const Vertex> vertices;
        std::span<const uint32_t> indices;

        size_t getIndexCount() const { return indices.size(); }
    };

    struct MeshGpuHandle
    {
        uint32_t vao = 0, vbo = 0, ebo = 0;
        uint32_t indexCount = 0;
    };
}
