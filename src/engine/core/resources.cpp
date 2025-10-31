#include "resources.h"

#include <fstream>
#include <spdlog/spdlog.h>

namespace res::shader_intl
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
            std::string infoLog;
            GLint infoLogLength;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
            infoLog.resize(infoLogLength);

            glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLog.data());
            spdlog::error("Shader compilation failed:\n{}", infoLog);
            glDeleteShader(shaderId);
            return { };
        }

        return shaderId;
    }

    std::optional<GLuint> compileShaderFromFile(const fs::path& path, GLenum shaderType)
    {
        std::optional<std::string> source = readFileText(path);
        if (!source.has_value())
        {
            // Propagate no value optional to caller
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
            std::string infoLog;
            GLint infoLogLength;
            glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
            infoLog.resize(infoLogLength);

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
        constexpr std::string_view ERROR_SHADER_VERT = "#version 330 core\nin vec3 pos;void main() {gl_Position = vec4(pos, 1.0);}";
        constexpr std::string_view ERROR_SHADER_FRAG = "#version 330 core\nout vec4 fragColor;void main() {fragColor = vec4(1.0, 0.0, 1.0, 1.0);}";

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

namespace
{
    struct DefaultResources
    {
        GLuint errorShader;
    };

    DefaultResources _defaults;
}

void res::initResources()
{
    _defaults.errorShader = shader_intl::compileErrorShader();
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
        spdlog::error("Could not find file {}", path.string());
        return { };
    }

    const auto size = file.tellg();
    std::string text = std::string(size, '\0');
    file.seekg(0);
    file.read(text.data(), size);
    return text;
}

render::shader::ShaderProgramData res::loadShader(const fs::path& vertPath, const fs::path& fragPath)
{
    std::optional<GLuint> vert = shader_intl::compileShaderFromFile(vertPath, GL_VERTEX_SHADER);
    std::optional<GLuint> frag = shader_intl::compileShaderFromFile(fragPath, GL_FRAGMENT_SHADER);

    if (vert && frag)
    {
        GLuint sId = shader_intl::makeShaderProgram({vert.value(), frag.value()}).value_or(_defaults.errorShader);
        return render::shader::ShaderProgramData{sId};
    }
    return render::shader::ShaderProgramData{_defaults.errorShader};
}
