#include "MeshGpuAllocator.h"

#include <glad/gl.h>

core::gpu::MeshGpuHandle core::gpu::MeshGpuAllocator::uploadMesh(const Mesh& mesh)
{
    return uploadMeshView(mesh.vertices, mesh.indices);
}

core::gpu::MeshGpuHandle core::gpu::MeshGpuAllocator::uploadMeshView(std::span<const Vertex> vertices, std::span<const u32> indices)
{
    gl::buffer_t vbo, ebo;
    gl::vert_arr_t vao;

    glCreateVertexArrays(1, &vao.id);

    glCreateBuffers(1, &vbo.id);
    glNamedBufferStorage(vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);

    glCreateBuffers(1, &ebo.id);
    glNamedBufferStorage(ebo, indices.size() * sizeof(u32), indices.data(), 0);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(vao, ebo);

    // Vertex.position
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribBinding(vao, 0, 0);

    // Vertex.normal
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribBinding(vao, 1, 0);

    // Vertex.uv0
    glEnableVertexArrayAttrib(vao, 2);
    glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv0));
    glVertexArrayAttribBinding(vao, 2, 0);

    MeshGpuHandle handle;
    handle.vao = vao;
    handle.vbo = vbo;
    handle.ebo = ebo;
    handle.indexCount = indices.size();
    return handle;
}

void core::gpu::MeshGpuAllocator::destroyMesh(MeshGpuHandle& handle)
{
    glDeleteVertexArrays(1, &handle.vao.id);
    glDeleteBuffers(1, &handle.vbo.id);
    glDeleteBuffers(1, &handle.ebo.id);

    handle = MeshGpuHandle(); // Null the old handle
}
