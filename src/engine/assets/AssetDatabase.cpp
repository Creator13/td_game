#include "AssetDatabase.h"

#include <optional>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "assets/File.h"
#include "rendering/shader.h"
#include "rendering/Mesh.h"
#include "assets/MeshPrimitives.h"
#include "assets/GltfElementTraits.h"

namespace fs = std::filesystem;

namespace
{
    constexpr std::string_view RUNTIME_PATH = "@runtime";
    constexpr std::string_view INTERNAL_PATH = "@internal";

    const assets::AssetId ERROR_SHADER_ID = assets::idFromPath("@internal/shader/error");
    const assets::AssetId DEBUG_SHADER_ID = assets::idFromPath("@internal/shader/debug");

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

    namespace _shader
    {
        std::optional<GLuint> compileShader(std::string_view source, GLenum shaderType);
        std::optional<GLuint> makeShaderProgram(std::initializer_list<GLuint> shaderIds);
        GLuint compileErrorShader();
    }

    namespace _shader
    {
        std::optional<GLuint> compileShader(std::string_view source, GLenum shaderType)
        {
            const GLuint shaderId = glCreateShader(shaderType);

            const char* sourcePtr = source.data();
            const GLint srcLength = static_cast<GLint>(source.length());
            glShaderSource(shaderId, 1, &sourcePtr, &srcLength);

            glCompileShader(shaderId);

            int success;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                GLint infoLogLength;
                glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::string infoLog = std::string(infoLogLength, '\0');

                glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLog.data());
                spdlog::error("Shader compilation failed:\n{}", infoLog);
                glDeleteShader(shaderId);
                return { };
            }

            return shaderId;
        }

        std::optional<GLuint> makeShaderProgram(std::initializer_list<GLuint> shaderIds)
        {
            const GLuint programId = glCreateProgram();
            for (const GLuint shaderId : shaderIds)
            {
                glAttachShader(programId, shaderId);
            }
            glLinkProgram(programId);

            int success;
            glGetProgramiv(programId, GL_LINK_STATUS, &success);
            if (!success)
            {
                GLint infoLogLength;
                glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::string infoLog = std::string(infoLogLength, '\0');

                glGetProgramInfoLog(programId, infoLogLength, nullptr, infoLog.data());

                spdlog::error("Shader linking failed:\n{}", infoLog);
                return { };
            }

            return programId;
        }

        GLuint compileErrorShader()
        {
            // The error shader compilation has no error checks; it assumes the error shader code remains valid and unchanged.
            constexpr std::string_view ERROR_SHADER_VERT = "#version 430\n layout (location = 0) in vec3 pos;\nlayout (location = 1) in vec3 aNorm;\nlayout (location = 100) uniform mat4 model;\nlayout (location = 101) uniform mat4 vp_mat;void main() {gl_Position = vp_mat * model * vec4(pos, 1.0);}";
            constexpr std::string_view ERROR_SHADER_FRAG = "#version 430\n out vec4 fragColor;void main() {fragColor = vec4(1.0, 0.0, 1.0, 1.0);}";

            GLuint vId = compileShader(ERROR_SHADER_VERT, GL_VERTEX_SHADER).value();
            GLuint fId = compileShader(ERROR_SHADER_FRAG, GL_FRAGMENT_SHADER).value();
            GLuint pId = makeShaderProgram({vId, fId}).value();

            glDeleteShader(vId);
            glDeleteShader(fId);

            return pId;
        }

        GLuint compileDebugShader()
        {
            constexpr std::string_view DEBUG_SHADER_VERT = "#version 430\n layout (location = 0) in vec3 pos;\nlayout (location = 101) uniform mat4 vp_mat;void main() {gl_Position = vp_mat * model * vec4(pos, 1.0);}";
            constexpr std::string_view DEBUG_SHADER_FRAG = "";

            GLuint vId = compileShader(DEBUG_SHADER_VERT, GL_VERTEX_SHADER).value();
            GLuint fId = compileShader(DEBUG_SHADER_FRAG, GL_FRAGMENT_SHADER).value();
            GLuint pId = makeShaderProgram({vId, fId}).value_or(0);

            glDeleteShader(vId);
            glDeleteShader(fId);

            return pId;
        }
    }

    namespace _mesh
    {
        constexpr math::vec3 transformGltfToEngineCoordinateSpace(const math::vec3& in)
        {
            return math::vec3(in.x, -in.z, in.y);
        }
    }
}

assets::AssetDatabase::AssetDatabase(std::string_view resourceRoot)
    : rootPath(file::getExecutableDir() / resourceRoot),
      metadata(128),
      meshes(64), meshAllocator(64),
      shaders(64)
{
    gltfParser = std::make_unique<fastgltf::Parser>();

    loadInternalMeshes();
}

assets::AssetDatabase::~AssetDatabase()
{
    meshAllocator.clean();
}

const assets::AssetInfo& assets::AssetDatabase::getAssetInfo(AssetId id)
{
    assert(metadata.contains(id));
    // Returns a copy; no outside influence is allowed // EDIT does it really?? const&?
    return metadata.at(id);
}

const graphics::MeshGpuHandle& assets::AssetDatabase::getMeshGpuHandle(AssetId id) const
{
    assert(metadata.contains(id));
    return meshAllocator.get(id);
}

const graphics::Mesh& assets::AssetDatabase::getMeshView(AssetId id) const
{
    assert(metadata.contains(id));
    return meshes.at(id);
}

graphics::Mesh& assets::AssetDatabase::getMeshMut(AssetId id)
{
    assert(metadata.contains(id));
    return meshes.at(id);
}

assets::AssetId assets::AssetDatabase::loadMeshFromFile(std::string_view path)
{
    AssetId id = idFromPath(path);
    if (meshes.contains(id)) { return id; }

    fs::path absPath = fs::path(resolveResourcePath(path));

    auto data = fastgltf::GltfDataBuffer::FromPath(absPath);
    if (data.error() != fastgltf::Error::None)
    {
        spdlog::error("Error loading gltf/glb file at {}: {}::{}", path, getErrorName(data.error()), getErrorMessage(data.error()));
        return 0;
    }

    constexpr fastgltf::Options parserOptions = fastgltf::Options::LoadExternalBuffers;

    auto load = gltfParser->loadGltfBinary(data.get(), absPath.parent_path(), parserOptions);
    if (auto error = load.error(); error != fastgltf::Error::None)
    {
        spdlog::error("Error parsing gltf/glb file at {}: {}::{}", path, getErrorName(data.error()), getErrorMessage(data.error()));
    }

    fastgltf::Asset asset = std::move(load.get());

    const fastgltf::Mesh& mesh = asset.meshes[0];

    graphics::Mesh out;

    size_t vertex_base = 0;

    for (const auto& primitive : mesh.primitives)
    {
        // position
        auto posAttribute = primitive.findAttribute("POSITION");
        if (posAttribute == nullptr)
        {
            spdlog::error("No POSITION attribute (TODO add more info)"); // TODO add more info
            continue;
        }
        const fastgltf::Accessor& posAccessor = asset.accessors[posAttribute->accessorIndex];

        size_t count = posAccessor.count;
        out.vertices.resize(vertex_base + count);

        fastgltf::iterateAccessorWithIndex<math::vec3>(asset, posAccessor,
            [&](math::vec3 pos, size_t index)
            {
                graphics::Vertex v;
                v.position = _mesh::transformGltfToEngineCoordinateSpace(pos);
                v.normal = math::vec3(0.0f, 0.0f, 0.0f);
                out.vertices[vertex_base + index] = v;
            });


        // normals
        auto normalAttribute = primitive.findAttribute("NORMAL");
        if (normalAttribute)
        {
            const fastgltf::Accessor& normalAccessor = asset.accessors[normalAttribute->accessorIndex];
            fastgltf::iterateAccessorWithIndex<math::vec3>(asset, normalAccessor,
                [&](math::vec3 normal, size_t index)
                {
                    out.vertices[vertex_base + index].normal = _mesh::transformGltfToEngineCoordinateSpace(normal);
                });
        }

        if (primitive.indicesAccessor.has_value())
        {
            const fastgltf::Accessor& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
            out.indices.resize(indexAccessor.count);
            fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, out.indices.data());
        }

        vertex_base += count;
    }

    AssetInfo info;
    info.id = id;
    info.path = path;
    info.type = AssetInfo::AssetType::Mesh;
    info.isRuntime = false;

    metadata.insert({info.id, info});
    graphics::Mesh& new_mesh = meshes.try_emplace(info.id).first->second;
    new_mesh.vertices = std::move(out.vertices);
    new_mesh.indices = std::move(out.indices);

    meshAllocator.upload(id, new_mesh);

    return id;
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

const graphics::ShaderProgramData& assets::AssetDatabase::getShaderProgram(AssetId id) const
{
    return shaders.at(id);
}

assets::AssetId assets::AssetDatabase::loadShaderFromFiles(std::string_view vertPath, std::string_view fragPath)
{
    std::optional<GLuint> vert = loadShaderStageFromFile(vertPath, GL_VERTEX_SHADER);
    std::optional<GLuint> frag = loadShaderStageFromFile(fragPath, GL_FRAGMENT_SHADER);
    if (!(vert && frag)) return ERROR_SHADER_ID;

    std::optional<GLuint> sId = _shader::makeShaderProgram({vert.value(), frag.value()});
    if (!sId) return ERROR_SHADER_ID;

    AssetInfo info;
    info.path = fmt::format("{}|{}", vertPath, fragPath);
    info.id = idFromPath(info.path);
    info.isRuntime = true;
    info.type = AssetInfo::AssetType::Shader;

    metadata.insert({info.id, info});

    shaders.insert({info.id, {sId.value()}});

    return info.id;
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
        sizeof(Vertex), (void*) offsetof(Vertex, position));

    // Vertex.normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*) offsetof(Vertex, normal));

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

std::string assets::AssetDatabase::resolveResourcePath(std::string_view path) const
{
    return (rootPath / fs::path(path)).string();
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

void assets::AssetDatabase::loadInternalShaders()
{
    GLuint glId = _shader::compileErrorShader();
    AssetId id = idFromPath(makeInternalPath(AssetInfo::AssetType::Shader, "error"));
    shaders.insert({id, graphics::ShaderProgramData{glId}});
}

std::optional<GLuint> assets::AssetDatabase::loadShaderStageFromFile(std::string_view path, uint32_t stageType)
{
    AssetId id = idFromPath(path);
    // Check cache
    if (shaderStageCache.contains(id))
    {
        spdlog::debug("Cache hit for shader stage at {}", path);
        return shaderStageCache.at(id);
    }

    // Not cached -> load into cache
    fs::path fullPath = resolveResourcePath(path);
    spdlog::debug("Loading shader stage file from {}", fullPath.generic_string());

    std::optional<std::string> source = file::readFileText(fullPath);
    if (!source.has_value())
    {
        spdlog::error("Failed to load shader stage: {}", path);
        return { };
    }

    std::optional<GLuint> glShaderId = _shader::compileShader(source.value().data(), stageType);
    if (glShaderId.has_value())
    {
        shaderStageCache.insert({id, glShaderId.value()});
        return glShaderId;
    }
    return { };
}
