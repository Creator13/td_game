#include "ShaderLoader.h"

#include <filesystem>
#include <string>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include "assets/AssetRegistery.h"
#include "assets/File.h"

using namespace core;
using namespace core::assets;

gl::program_t ShaderLoader::compileErrorShader()
{
    // The error shader compilation has no error checks; it assumes the error shader code remains valid and unchanged.
    constexpr std::string_view ERROR_SHADER_VERT = "#version 430\n layout (location = 0) in vec3 pos;\nlayout (location = 1) in vec3 aNorm;\nlayout (location = 100) uniform mat4 model;\nlayout (location = 101) uniform mat4 vp_mat;void main() {gl_Position = vp_mat * model * vec4(pos, 1.0);}";
    constexpr std::string_view ERROR_SHADER_FRAG = "#version 430\n out vec4 fragColor;void main() {fragColor = vec4(1.0, 0.0, 1.0, 1.0);}";

    gl::shader_t vId = compileFromSource(ERROR_SHADER_VERT, GL_VERTEX_SHADER).value();
    gl::shader_t fId = compileFromSource(ERROR_SHADER_FRAG, GL_FRAGMENT_SHADER).value();
    const gl::program_t pId = linkShaderProgram({vId, fId}).value();

    glDeleteShader(vId);
    glDeleteShader(fId);

    return pId;
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
        spdlog::error("Shader compilation failed:\n{}", infoLog);
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

        spdlog::error("Shader linking failed:\n{}", infoLog);
        return std::nullopt;
    }

    return programId;
}

std::optional<gl::shader_t> ShaderLoader::loadShaderStageFromFile(std::string_view path, gl::enum_t stageType)
{
    AssetId id = AssetId::idFromPath(path);
    // Check cache
    if (shaderStageCache.contains(id))
    {
        spdlog::debug("Cache hit for shader stage at {}", path);
        return shaderStageCache.at(id);
    }

    // Not cached -> load into cache
    const file::fs::path fullPath = resolveResourcePath(path);
    spdlog::debug("Loading shader stage file from {}", fullPath.generic_string());

    const std::optional<std::string> source = file::readFileText(fullPath);
    if (!source.has_value())
    {
        spdlog::error("Failed to load shader stage: {}", path);
        return std::nullopt;
    }

    std::optional<gl::shader_t> glShaderId = compileFromSource(source.value().data(), stageType);
    if (glShaderId.has_value())
    {
        shaderStageCache.insert({id, glShaderId.value()});
        return glShaderId;
    }
    return std::nullopt;
}
