#include "ShaderLoader.h"

#include <filesystem>
#include <string>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include "assets/AssetDatabase.h"
#include "assets/File.h"
#include "rendering/DataLayout.h"

using namespace core;
using namespace core::assets;

gl::program_t ShaderLoader::compileInternalErrorShader()
{
    // The error shader compilation has no error checks; it assumes the error shader code remains valid and unchanged.
    constexpr std::string_view errorShaderVert = R"(
            #version 430 core

            layout (location = 0) in vec3 pos;
            layout (location = 1) in vec3 aNorm;

            void main() {
                gl_Position = scene.viewProj * object.worldTransform * vec4(pos, 1.0);
            }
        )";

    constexpr std::string_view errorShaderFrag = R"(
            #version 430 core

            out vec4 fragColor;

            void main() {
                fragColor = vec4(1.0, 0.0, 1.0, 1.0);
            }
        )";

    auto vId = compileFromSource(preprocessShader(errorShaderVert), GL_VERTEX_SHADER);
    auto fId = compileFromSource(preprocessShader(errorShaderFrag), GL_FRAGMENT_SHADER);
    if (!vId || !fId)
    {
        SPDLOG_ERROR("Failed to compile debug shader!");
        return 0;
    }

    const auto pId = linkShaderProgram({vId.value(), fId.value()});
    if (!pId)
    {
        SPDLOG_ERROR("Failed to link debug shader!");
        return 0;
    }

    glDeleteShader(vId.value());
    glDeleteShader(fId.value());

    return pId.value();
}

std::string injectSystemHeader(std::string_view src)
{
    std::string result{src};

    const usize versionPos = src.find("#version");
    if (versionPos != std::string::npos)
    {
        const usize nextLine = src.find('\n', versionPos) + 1;

        // For simplicity we keep inserting at the top, but for readability we
        // want the bindings to appear in order of value so we insert high to low.
        result.insert(nextLine, gfx::InstanceData::getShaderDeclaration()); // binding 1
        result.insert(nextLine, gfx::PassDataBlock::getShaderDeclaration()); // binding 0
    }
    return result;
}

std::string ShaderLoader::preprocessShader(std::string_view sourceString)
{
    std::string result = injectSystemHeader(sourceString);
    return result;
}

ShaderLoader::~ShaderLoader()
{
    cleanCache();
}

std::optional<gl::shader_t> ShaderLoader::compileFromSource(std::string_view source, gl::enum_t shaderType)
{
    gl::shader_t shaderId = glCreateShader(shaderType);

    const char* sourcePtr = source.data();
    const gl::Int srcLength = static_cast<gl::Int>(source.length());
    glShaderSource(shaderId, 1, &sourcePtr, &srcLength);

    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        gl::Int infoLogLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog = std::string(infoLogLength, '\0');

        glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLog.data());
        SPDLOG_CRITICAL("Shader compilation failed:\n{}", infoLog);
        glDeleteShader(shaderId);
        return std::nullopt;
    }

    return shaderId;
}

std::optional<gl::program_t> ShaderLoader::linkShaderProgram(std::initializer_list<gl::shader_t> shaderIds)
{
    const gl::program_t programId = glCreateProgram();
    for (const gl::shader_t shaderId : shaderIds)
    {
        glAttachShader(programId, shaderId);
    }
    glLinkProgram(programId);

    int success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        gl::Int infoLogLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog = std::string(infoLogLength, '\0');

        glGetProgramInfoLog(programId, infoLogLength, nullptr, infoLog.data());

        SPDLOG_ERROR("Shader linking failed:\n{}", infoLog);
        return std::nullopt;
    }

    return programId;
}

std::optional<gl::shader_t> ShaderLoader::loadShaderStageFromFile(std::string_view path, gl::enum_t stageType, bool preprocess)
{
    AssetId id = AssetId::idFromPath(path);
    // Check cache
    if (_shaderStageCache.contains(id))
    {
        SPDLOG_DEBUG("Cache hit for shader stage at {}", path);
        return _shaderStageCache.at(id);
    }

    // Not cached -> load into cache
    const file::fs::path fullPath = AssetDatabase::resolveResourcePath(path);
    SPDLOG_DEBUG("Loading shader stage file from {}", fullPath.generic_string());

    std::optional<std::string> source = file::readFileText(fullPath);
    if (!source.has_value())
    {
        SPDLOG_ERROR("Failed to load shader stage: {}", path);
        return std::nullopt;
    }

    std::optional<gl::shader_t> glShaderId = compileFromSource(preprocess ? preprocessShader(source.value()) : source.value(), stageType);
    if (glShaderId.has_value())
    {
        _shaderStageCache.insert({id, glShaderId.value()});
        return glShaderId;
    }
    return std::nullopt;
}

gl::program_t ShaderLoader::glProgramFromFiles(std::string_view vertPath, std::string_view fragPath)
{
    std::optional<gl::shader_t> vertId = loadShaderStageFromFile(vertPath, GL_VERTEX_SHADER, true);
    std::optional<gl::shader_t> fragId = loadShaderStageFromFile(fragPath, GL_FRAGMENT_SHADER, true);
    if (!vertId || !fragId)
    {
        return getErrorShader();
    }

    std::optional<gl::program_t> sId = linkShaderProgram({vertId.value(), fragId.value()});
    if (!sId) return getErrorShader();

    return sId.value();
}

gl::program_t ShaderLoader::getErrorShader()
{
    if (_errorShaderId == 0)
    {
        _errorShaderId = compileInternalErrorShader();
    }

    return _errorShaderId;
}

void ShaderLoader::cleanCache()
{
    for (auto& shaderId : _shaderStageCache | std::views::values)
    {
        glDeleteShader(shaderId);
    }
}
