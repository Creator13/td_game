#pragma once

#include <cstdint>
#include <vector>

#include "math/vec3.h"

namespace graphics
{
    struct Vertex
    {
        math::vec3 position;
        math::vec3 normal;
    };

    class Mesh
    {
        uint32_t vao, vbo, ebo;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

    public:
        Mesh();
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
        ~Mesh();

    private:
        void constructBuffers();

    public:
        size_t getIndexCount() const { return indices.size(); }
        void uploadToGpu();
    };
}
