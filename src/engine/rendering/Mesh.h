#pragma once

#include <vector>

#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "math/geom.h"
#include "math/vec3.h"
#include "rendering/MeshGpuHandle.h"

namespace core
{
    struct Mesh;

    template<>
    struct assets::AssetTraits<Mesh>
    {
        static constexpr AssetType type = AssetType::Mesh;
    };

    struct Vertex
    {
        math::vec3 position;
        math::vec3 normal;
    };

    struct Mesh
    {
        ~Mesh();

        std::vector<Vertex> vertices;
        std::vector<u32> indices;

        math::AABB bounds;
        gpu::MeshGpuHandle gpuHandle;

        void recalculateBounds();

        static assets::AssetRef<Mesh> loadFromFile(std::string_view path);
        static assets::AssetRef<Mesh> create();

    private:
        Mesh() = default;
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
