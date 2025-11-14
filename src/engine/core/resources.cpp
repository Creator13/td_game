#include "resources.h"

#include <fstream>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace
{
    namespace fs = std::filesystem;

    namespace _shader
    {
        std::optional<GLuint> compileShader(const std::string_view& source, GLenum shaderType);
        std::optional<GLuint> compileShaderFromFile(const fs::path& path, GLenum shaderType);
        std::optional<GLuint> makeShaderProgram(std::initializer_list<GLuint> shaderIds);
        GLuint compileErrorShader();
    }

    namespace _file
    {
        fs::path getExecutableDir();
        fs::path resolveResource(const fs::path& path);
    }

    namespace _shader
    {
        std::optional<GLuint> compileShader(const std::string_view& source, GLenum shaderType)
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

        std::optional<GLuint> compileShaderFromFile(const fs::path& path, GLenum shaderType)
        {
            fs::path fullPath = _file::resolveResource(path);
            spdlog::debug("Loading shader file from {}", fullPath.generic_string());

            std::optional<std::string> source = res::readFileText(fullPath);
            if (!source.has_value())
            {
                spdlog::error("Failed to load shader: {}", path.string());
                return { };
            }

            return compileShader(source.value(), shaderType);
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
            constexpr std::string_view ERROR_SHADER_VERT = "#version 330\nin vec3 pos;void main() {gl_Position = vec4(pos, 1.0);}";
            constexpr std::string_view ERROR_SHADER_FRAG = "#version 330\nout vec4 fragColor;void main() {fragColor = vec4(1.0, 0.0, 1.0, 1.0);}";

            GLuint vId = glCreateShader(GL_VERTEX_SHADER);
            const char* vSrc = ERROR_SHADER_VERT.data();
            const GLint vLen = static_cast<GLint>(ERROR_SHADER_VERT.length());
            glShaderSource(vId, 1, &vSrc, &vLen);
            glCompileShader(vId);

            GLuint fId = glCreateShader(GL_FRAGMENT_SHADER);
            const char* fSrc = ERROR_SHADER_FRAG.data();
            const GLint fLen = static_cast<GLint>(ERROR_SHADER_FRAG.length());
            glShaderSource(fId, 1, &fSrc, &fLen);
            glCompileShader(fId);

            GLuint pId = glCreateProgram();
            glAttachShader(pId, vId);
            glAttachShader(pId, fId);
            glLinkProgram(pId);

            glDeleteShader(vId);
            glDeleteShader(fId);

            return pId;
        }
    }

    namespace _file
    {
        fs::path rootPath;

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

        fs::path resolveResource(const fs::path& relPath)
        {
            return rootPath / relPath;
        }
    }

    struct DefaultResources
    {
        GLuint errorShader;
    };

    DefaultResources _defaults;
}

void res::initResources(std::string_view resourceRoot)
{
    _file::rootPath = _file::getExecutableDir() / resourceRoot;

    _defaults.errorShader = _shader::compileErrorShader();
}

void res::unloadResources()
{
    glDeleteProgram(_defaults.errorShader);
}

std::optional<std::string> res::readFileText(const fs::path& path)
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

graphics::shader::ShaderProgramData res::loadShader(const fs::path& vertPath, const fs::path& fragPath)
{
    std::optional<GLuint> vert = _shader::compileShaderFromFile(vertPath, GL_VERTEX_SHADER);
    std::optional<GLuint> frag = _shader::compileShaderFromFile(fragPath, GL_FRAGMENT_SHADER);

    if (vert && frag)
    {
        GLuint sId = _shader::makeShaderProgram({vert.value(), frag.value()}).value_or(_defaults.errorShader);
        return graphics::shader::ShaderProgramData{sId};
    }
    return graphics::shader::ShaderProgramData{_defaults.errorShader};
}
