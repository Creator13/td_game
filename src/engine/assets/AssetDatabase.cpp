#include "AssetDatabase.h"

#include <fstream>
#include <optional>
#include <xxhash.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "rendering/shader.h"
#include "rendering/Mesh.h"
#include "assets/MeshPrimitives.h"


namespace fs = std::filesystem;

namespace
{
    constexpr std::string_view RUNTIME_PATH = "@runtime";
    constexpr std::string_view INTERNAL_PATH = "@internal";

    const assets::AssetId ERROR_SHADER_ID = assets::AssetDatabase::idFromPath("@internal/shader/error");

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

    namespace _file
    {
        fs::path getExecutableDir();
        std::optional<std::string> readFileText(const fs::path& path);
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
            for (const GLuint shaderId: shaderIds)
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

            for (const GLuint shaderId: shaderIds)
            {
                glDeleteShader(shaderId);
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
    }

    namespace _file
    {
        fs::path getExecutableDir()
        {
#ifdef _WIN32
            wchar_t buffer[1024];
            GetModuleFileNameW(nullptr, buffer, 1024);
            return fs::path(buffer).parent_path();
#else // Linux
            char buffer[1024];
            ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (len != -1)
            {
                buffer[len] = '\0';
                return fs::path(buffer).parent_path();
            }
            else
            {
                throw std::runtime_error("Failed to get executable path");
            }
#endif
        }

        std::optional<std::string> readFileText(const fs::path& path)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file)
            {
                return { };
            }

            const auto size = file.tellg();
            std::string text = std::string(size, '\0');
            file.seekg(0);
            file.read(text.data(), size);
            return text;
        }
    }
}

assets::AssetDatabase::AssetDatabase(std::string_view resourceRoot)
    : rootPath(_file::getExecutableDir() / resourceRoot),
      metadata(128),
      meshes(64), meshAllocator(),
      shaders(64)
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
    for (auto& [id, handle]: handles)
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

    std::optional<std::string> source = _file::readFileText(fullPath);
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
