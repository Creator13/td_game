#include "AssetDatabase.h"

#include "Logging.h"
#include "assets/File.h"
#include "assets/Shader.h"
#include "core/Assert.h"

namespace fs = std::filesystem;

using namespace core;
using namespace core::assets;

namespace
{
    constexpr std::string_view RUNTIME_PATH = "@runtime";
    constexpr std::string_view INTERNAL_PATH = "@internal";

    const AssetId ERROR_SHADER_ID = AssetId::idFromPath("@internal/shader/error");

    constexpr std::string_view mapAssetTypeName(AssetType type)
    {
        switch (type)
        {
            case AssetType::Mesh:
                return "mesh";
            case AssetType::Shader:
                return "shader";
            default:
                return "misc";
        }
    }
}

AssetDatabase::AssetDatabase(std::string_view resourceRoot)
    : _rootPath(file::getExecutableDir() / resourceRoot)
{

}

const AssetInfo& AssetDatabase::getAssetInfo(AssetId id)
{
    ENGINE_ASSERT(metadata.contains(id), "Asset not found: {}", id);
    // Returns a copy; no outside influence is allowed // EDIT does it really?? const&?
    return metadata.at(id);
}

// AssetId AssetDatabase::createMesh(std::string_view path)
// {
//     using namespace graphics;
//
//     AssetInfo info;
//     info.path = path;
//     info.id = idFromPath(info.path);
//     info.isRuntime = true;
//     info.type = AssetInfo::AssetType::Mesh;
//
//     ENGINE_ASSERT(!metadata.contains(info.id), "AssetDatabase already contains an asset at path {}", path);
//     metadata.insert({info.id, info});
//
//     meshes.try_emplace(info.id);
//
//     return info.id;
// }
//
// AssetId AssetDatabase::createRuntimeMesh(std::string_view name)
// {
//     return globalAssetDatabase->createMesh(fmt::format("{}/{}/{}", RUNTIME_PATH, mapAssetTypeName(AssetInfo::AssetType::Mesh), name));
// }
//
// AssetId AssetDatabase::loadTextureFromFile(std::string_view path)
// {
//     const AssetId id = idFromPath(path);
//     ENGINE_ASSERT(!globalAssetDatabase->metadata.contains(id), "Asset already loaded: {}", path);
//
//     uint32_t texture;
//     glCreateTextures(GL_TEXTURE_2D, 1, &texture);
//
//     glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//     int width, height, channels;
//     const uint8_t* data = stbi_load(path.data(), &width, &height, &channels, 0);
//
//     if (data)
//     {
//         glTextureStorage2D(texture, 1, GL_RGB8, width, height);
//         glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
//     }
//     else
//     {
//         spdlog::error("Failed to load texture at path: {}", path);
//         return 0;
//     }
//
//     AssetInfo info;
//     info.path = path;
//     info.id = id;
//     info.isRuntime = true;
//     info.type = AssetInfo::AssetType::Texture;
//
//     return id;
// }
//
// AssetId AssetDatabase::loadShaderFromFiles(std::string_view vertPath, std::string_view fragPath)
// {
//     std::optional<GLuint> vert = globalAssetDatabase->loadShaderStageFromFile(vertPath, GL_VERTEX_SHADER);
//     std::optional<GLuint> frag = globalAssetDatabase->loadShaderStageFromFile(fragPath, GL_FRAGMENT_SHADER);
//     if (!(vert && frag)) return ERROR_SHADER_ID;
//
//     std::optional<GLuint> sId = _shader::makeShaderProgram({vert.value(), frag.value()});
//     if (!sId) return ERROR_SHADER_ID;
//
//     AssetInfo info;
//     info.path = fmt::format("{}|{}", vertPath, fragPath);
//     info.id = idFromPath(info.path);
//     info.isRuntime = true;
//     info.type = AssetInfo::AssetType::Shader;
//
//     globalAssetDatabase->metadata.insert({info.id, info});
//     globalAssetDatabase->shaders.insert({info.id, {sId.value()}});
//
//     return info.id;
// }
//
// const MeshGpuHandle& AssetDatabase::MeshGpuAllocator::get(AssetId id) const
// {
//     return handles.at(id);
// }
//
// void AssetDatabase::MeshGpuAllocator::upload(AssetId id, const Mesh& mesh)
// {
//     using namespace graphics;
//     MeshGpuHandle handle;
//
//     glGenBuffers(1, &handle.vbo);
//     glGenBuffers(1, &handle.ebo);
//     glGenVertexArrays(1, &handle.vao);
//
//     glBindVertexArray(handle.vao);
//
//     glBindBuffer(GL_ARRAY_BUFFER, handle.vbo);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
//
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ebo);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);
//
//     // Vertex.position
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(
//         0,
//         3, GL_FLOAT, GL_FALSE,
//         sizeof(Vertex), (void*) offsetof(Vertex, position));
//
//     // Vertex.normal
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(
//         1,
//         3, GL_FLOAT, GL_FALSE,
//         sizeof(Vertex), (void*) offsetof(Vertex, normal));
//
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//     handle.indexCount = mesh.indices.size();
//     handles.insert({id, handle});
// }
//
// void AssetDatabase::MeshGpuAllocator::clean()
// {
//     for (auto& [id, handle] : handles)
//     {
//         glDeleteVertexArrays(1, &handle.vao);
//         glDeleteBuffers(1, &handle.ebo);
//         glDeleteBuffers(1, &handle.vbo);
//     }
// }

fs::path AssetDatabase::resolveResourcePath(std::string_view path) const
{
    return _rootPath / fs::path(path);
}

std::string AssetDatabase::makeInternalPath(AssetType type, std::string_view name)
{
    std::string_view typePath = mapAssetTypeName(type);
    return fmt::format("{}/{}/{}", INTERNAL_PATH, typePath, name);
}

// void AssetDatabase::loadInternalMesh(
//     std::string_view name,
//     const Vertex* vPtr, uint32_t vCnt,
//     const uint32_t* iPtr, uint32_t iCnt,
//     const math::AABB& bounds)
// {
//     std::string path = makeInternalPath(AssetInfo::AssetType::Mesh, name);
//     AssetInfo info;
//     info.id = idFromPath(path);
//     info.path = std::move(path);
//     info.isRuntime = false;
//     info.type = AssetInfo::AssetType::Mesh;
//
//     metadata.insert({info.id, info});
//
//     meshes.try_emplace(info.id);
//     Mesh& mesh = meshes.at(info.id);
//     mesh.vertices = std::vector(vPtr, vPtr + vCnt);
//     mesh.indices = std::vector(iPtr, iPtr + iCnt);
//     mesh.bounds = bounds;
//
//     meshAllocator.upload(info.id, mesh);
// }
//
// void AssetDatabase::loadInternalMeshes()
// {
//     using namespace mesh_primitives;
//     loadInternalMesh("cube", CUBE_VERTICES, 24, CUBE_INDICES, 36, CUBE_BOUNDS);
//     loadInternalMesh("quad", QUAD_VERTICES, 4, QUAD_INDICES, 6, QUAD_BOUNDS);
// }
