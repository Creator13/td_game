#include "ShaderLoader.h"

#include <unordered_set>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include "assets/AssetDatabase.h"
#include "assets/File.h"
#include "rendering/DataLayout.h"
#include "util/StringExtensions.h"

using namespace core;
using namespace core::assets;

namespace
{
    namespace fs = std::filesystem;

    struct PreprocessorState
    {
        static constexpr int MAX_DEPTH = 32;

        std::string rootVersion;
        std::unordered_set<std::string> includedFiles;
        std::vector<std::string> includeStack;
        std::unordered_map<std::string, int> fileIds;
        int nextFileId = 0;

        int getFileId(const std::string& path)
        {
            if (!fileIds.contains(path))
            {
                fileIds[path] = nextFileId++;
            }
            return fileIds[path];
        }
    };

    std::string processIncludes(std::string_view source, const fs::path& currentPath, PreprocessorState& state, int depth)
    {
        if (depth >= PreprocessorState::MAX_DEPTH)
        {
            SPDLOG_ERROR("Max include depth reached at: {}", currentPath.generic_string());
            return "";
        }

        std::string result;
        result.reserve(source.size() * 1.1); // Why 1.1

        const int currentFileId = state.getFileId(currentPath.generic_string());

        if (depth > 0)
        {
            result += std::format("#line 1 {}\n", currentFileId);
        }

        usize lineNumber = 1;
        usize start = 0;

        while (start < source.size())
        {
            usize end = source.find("\n", start);
            const bool hasNewLine = end != std::string_view::npos;
            std::string_view line = source.substr(start, hasNewLine ? end - start : std::string_view::npos);
            std::string_view trimmed = util::string::trimWhitespace(line);

            if (trimmed.starts_with("#version"))
            {
                std::string_view versionInfo = util::string::trimWhitespace(trimmed.substr(8));
                if (state.rootVersion.empty()) // Arguably this should be checked against the current depth?
                {
                    state.rootVersion = std::string(versionInfo);
                    result += std::string(line) + (hasNewLine ? "\n" : "");
                }
                else
                {
                    if (state.rootVersion != versionInfo)
                    {
                        SPDLOG_WARN("Shader version mismatch in {}: expected {}, got {}.", currentPath.generic_string(), state.rootVersion, versionInfo);
                    }
                    result += "\n";
                    // This will result in the file still being written, right? We should exit but only if the versions are likely to be incompatible (eg a different profile)
                }
            }
            else if (trimmed.starts_with("#include"))
            {
                const usize firstQuotePos = trimmed.find('\"');
                const usize lastQuotePos = trimmed.rfind('\"');

                if (firstQuotePos != std::string_view::npos && lastQuotePos != std::string_view::npos && firstQuotePos < lastQuotePos)
                {
                    std::string_view includePathStr = trimmed.substr(firstQuotePos + 1, lastQuotePos - firstQuotePos - 1);
                    fs::path includePath = currentPath.parent_path() / includePathStr;
                    includePath = includePath.lexically_normal();
                    std::string includePathNorm = includePath.generic_string();

                    if (std::ranges::contains(state.includeStack, includePathNorm))
                    {
                        SPDLOG_ERROR("Cyclic include detected: {}", includePathNorm); // TODO this gives no usable information...
                        result += "\n";
                    }
                    // Check if already included
                    else if (state.includedFiles.contains(includePathNorm))
                    {
                        result += "\n";
                    }
                    else
                    {
                        state.includedFiles.insert(includePathNorm);
                        state.includeStack.push_back(includePathNorm);

                        if (auto includeSource = file::readFileText(includePath))
                        {
                            result += processIncludes(includeSource.value(), includePath, state, depth + 1);
                            result += fmt::format("\n#line {} {}\n", lineNumber + 1, currentFileId);
                        }
                        else
                        {
                            SPDLOG_ERROR("Failed to read file at #include {}", includePathNorm); // TODO should also include root path probably
                        }

                        state.includeStack.pop_back();
                    }
                }
                else
                {
                    SPDLOG_WARN("Malformed #include directive in {}: {}", currentPath.generic_string(), line);
                    result += "\n";
                }
            }
            else
            {
                result += std::string(line) + (hasNewLine ? "\n" : "");
            }

            // Regular line, just append
            start = hasNewLine ? end + 1 : source.size();
            lineNumber++;
        }

        return result;
    }
}

gl::program_t ShaderLoader::compileInternalErrorShader()
{
    // The error shader compilation has no error checks; it assumes the error shader code remains valid and unchanged.
    constexpr std::string_view errorShaderVert = R"(
            #version 430 core

            struct CustomData {
                uint c0, c1, c2, c3;
            };

            struct InstanceData {
                mat4 transform;
                CustomData customData;
            };

            layout (std430, binding = 2) readonly buffer instanceSSBO {
                InstanceData instances[];
            };

            #define INSTANCE_TRANSFORM (instances[gl_BaseInstance + gl_InstanceID].transform)
            #define INSTANCE_DATA (instances[gl_BaseInstance + gl_InstanceID].customData)

            layout (binding = 0, std140) uniform ViewportDataBlock
            {
                mat4 view;
                mat4 projection;
                mat4 viewProj;
            } scene;

            layout (binding = 1, std140) uniform FrameDataBlock {
                vec2 screenSize;
                float currentTime;
            } frame;

            layout (location = 0) in vec3 pos;
            layout (location = 1) in vec3 aNorm;

            void main() {
                gl_Position = scene.viewProj * INSTANCE_TRANSFORM * vec4(pos, 1.0);
            }
        )";

    constexpr std::string_view errorShaderFrag = R"(
            #version 430 core

            out vec4 fragColor;

            void main() {
                fragColor = vec4(1.0, 0.0, 1.0, 1.0);
            }
        )";

    auto vId = compileFromSource(errorShaderVert, GL_VERTEX_SHADER);
    auto fId = compileFromSource(errorShaderFrag, GL_FRAGMENT_SHADER);
    if (!vId || !fId)
    {
        SPDLOG_ERROR("Failed to compile error shader!");
        return 0;
    }

    const auto pId = linkShaderProgram({vId.value(), fId.value()});
    if (!pId)
    {
        SPDLOG_ERROR("Failed to link error shader!");
        return 0;
    }

    glDeleteShader(vId.value());
    glDeleteShader(fId.value());

    return pId.value();
}

std::optional<std::string> ShaderLoader::preprocessShader(const fs::path& sourcePath)
{
    const auto fileText = file::readFileText(sourcePath);
    if (!fileText)
    {
        SPDLOG_ERROR("Failed to read text from file at {}", sourcePath.generic_string());
        return std::nullopt;
    }

    PreprocessorState state;
    state.includedFiles.insert(sourcePath.generic_string());
    state.includeStack.push_back(sourcePath.generic_string());

    return processIncludes(fileText.value(), sourcePath, state, 0);
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

    std::string source;
    if (preprocess)
    {
        const auto processedOpt = preprocessShader(fullPath);
        if (!processedOpt.has_value())
        {
            // TODO this is ugly because we have duplicated code in two paths that still requires this branch to pass on the nullopt. But we don't want to keep passing the path along either...
            return std::nullopt;
        }
        else
        {
            source = processedOpt.value();
        }
    }
    else
    {
        const auto fileText = file::readFileText(fullPath);
        if (!fileText)
        {
            SPDLOG_ERROR("Failed to read text from file at {}", path);
            return std::nullopt;
        }
        source = fileText.value();
    }

    std::optional<gl::shader_t> glShaderId = compileFromSource(source, stageType);
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
