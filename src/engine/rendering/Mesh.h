#pragma once

#include <cstdint>
#include <vector>

#include "math/geom.h"
#include "math/vec3.h"

namespace core
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

        math::AABB bounds;

        void recalculateBounds();
    };

    struct MeshGpuHandle
    {
        uint32_t vao = 0, vbo = 0, ebo = 0;
        uint32_t indexCount = 0;
    };

    template<std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, Vertex>
    constexpr math::AABB boundsFromVertices(R&& vertices)
    {
        math::vec3 min = std::numeric_limits<math::vec3>::max();
        math::vec3 max = std::numeric_limits<math::vec3>::lowest();

        for (const Vertex& v : vertices)
        {
            min = comptMin(v.position, min);
            max = comptMax(v.position, max);
        }

        return math::AABB::fromMinMax(min, max);
    }
}
