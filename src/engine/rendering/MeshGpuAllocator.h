#pragma once

#include "Mesh.h"
#include "MeshGpuHandle.h"

namespace core::gpu
{
    class MeshGpuAllocator
    {
    public:
        MeshGpuHandle uploadMesh(const Mesh& mesh);
        void destroyMesh(MeshGpuHandle& handle);
    };
}
