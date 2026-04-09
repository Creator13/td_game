#pragma once

#include "assets/Mesh.h"
#include "rendering/MeshGpuHandle.h"

namespace core::gpu
{
    class MeshGpuAllocator
    {
    public:
        MeshGpuHandle uploadMesh(const Mesh& mesh);
        static MeshGpuHandle uploadMeshView(std::span<const Vertex> vertices, std::span<const u32> indices);
        void destroyMesh(MeshGpuHandle& handle);
    };
}
