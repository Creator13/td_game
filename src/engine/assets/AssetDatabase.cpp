#include "AssetDatabase.h"

#include <xxhash.h>
#include <glad/glad.h>
#include <spdlog/fmt/fmt.h>

#include "rendering/shader.h"
#include "rendering/Mesh.h"
#include "assets/MeshPrimitives.h"

constexpr std::string_view RUNTIME_PATH = "@runtime";
constexpr std::string_view INTERNAL_PATH = "@internal";

constexpr std::string_view mapAssetTypeName(assets::AssetInfo::AssetType type)
{
    switch (type)
    {
        case assets::AssetInfo::AssetType::Mesh:
            return "mesh";
        case assets::AssetInfo::AssetType::Shader:
            return "shader";
        default:
            return "misc";
    }
}

assets::AssetDatabase::AssetDatabase(std::string_view resourceRoot)
    : resourceRoot(resourceRoot),
      metadata(),
      meshes(), meshAllocator(),
      shaders()
{
    loadInternalMeshes();
}

assets::AssetDatabase::~AssetDatabase()
{
    meshAllocator.clean();
}

assets::AssetId assets::AssetDatabase::idFromPath(std::string_view path)
{
    return static_cast<AssetId>(XXH3_64bits(path.data(), path.size()));
}

const assets::AssetInfo& assets::AssetDatabase::getAssetInfo(AssetId id)
{
    assert(metadata.contains(id));
    // Returns a copy; no outside influence is allowed
    return metadata.at(id);
}

const graphics::MeshGpuHandle& assets::AssetDatabase::getMeshGpuHandle(AssetId id) const
{
    return meshAllocator.get(id);
}

const graphics::Mesh& assets::AssetDatabase::getMeshView(AssetId id) const
{
    return meshes.at(id);
}

graphics::Mesh& assets::AssetDatabase::getMeshMut(AssetId id)
{
    return meshes.at(id);
}

assets::AssetId assets::AssetDatabase::loadMeshFromFile(std::string_view path)
{
    return 0;
}

assets::AssetId assets::AssetDatabase::createMesh(std::string_view path)
{
    using namespace graphics;

    AssetInfo info;
    info.path = path;
    info.id = idFromPath(info.path);
    info.isRuntime = true;
    info.type = AssetInfo::AssetType::Mesh;

    assert(!metadata.contains(info.id));
    metadata.insert({info.id, info});

    meshes.try_emplace(info.id);

    return info.id;
}

assets::AssetId assets::AssetDatabase::createRuntimeMesh(std::string_view name)
{
    return createMesh(fmt::format("{}/{}/{}", RUNTIME_PATH, mapAssetTypeName(AssetInfo::AssetType::Mesh), name));
}

const graphics::MeshGpuHandle& assets::AssetDatabase::MeshGpuAllocator::get(AssetId id) const
{
    return handles.at(id);
}

void assets::AssetDatabase::MeshGpuAllocator::upload(AssetId id, const graphics::Mesh& mesh)
{
    using namespace graphics;
    MeshGpuHandle handle;

    glGenBuffers(1, &handle.vbo);
    glGenBuffers(1, &handle.ebo);
    glGenVertexArrays(1, &handle.vao);

    glBindVertexArray(handle.vao);

    glBindBuffer(GL_ARRAY_BUFFER, handle.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);

    // Vertex.position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Vertex.normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    handle.indexCount = mesh.indices.size();
    handles.insert({id, handle});
}

void assets::AssetDatabase::MeshGpuAllocator::clean()
{
    for (auto& [id, handle] : handles)
    {
        glDeleteVertexArrays(1, &handle.vao);
        glDeleteBuffers(1, &handle.ebo);
        glDeleteBuffers(1, &handle.vbo);
    }
}

std::string assets::AssetDatabase::makeInternalPath(AssetInfo::AssetType type, std::string_view name)
{
    std::string_view typePath = mapAssetTypeName(type);
    return fmt::format("{}/{}/{}", INTERNAL_PATH, typePath, name);
}

void assets::AssetDatabase::loadInternalMesh(
    std::string_view name,
    const graphics::Vertex* vPtr, uint32_t vCnt,
    const uint32_t* iPtr, uint32_t iCnt)
{
    std::string path = makeInternalPath(AssetInfo::AssetType::Mesh, name);
    AssetInfo info;
    info.id = idFromPath(path);
    info.path = std::move(path);
    info.isRuntime = false;
    info.type = AssetInfo::AssetType::Mesh;

    metadata.insert({info.id, info});

    meshes.try_emplace(info.id);
    graphics::Mesh& mesh = meshes.at(info.id);
    mesh.vertices = std::vector(vPtr, vPtr + vCnt);
    mesh.indices = std::vector(iPtr, iPtr + iCnt);

    meshAllocator.upload(info.id, mesh);
}

void assets::AssetDatabase::loadInternalMeshes()
{
    using namespace mesh_primitives;
    loadInternalMesh("cube", CUBE_VERTICES, 24, CUBE_INDICES, 36);
    loadInternalMesh("quad", QUAD_VERTICES, 4, QUAD_INDICES, 6);
}
