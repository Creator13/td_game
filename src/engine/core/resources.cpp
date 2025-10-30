#include "resources.h"

#include <fstream>
#include <spdlog/spdlog.h>

constexpr uint32_t INFO_BUF_SIZE = 512;

namespace res::shader_intl
{
    using namespace render::shader;

    constexpr std::string_view ERROR_SHADER_VERT = "#version 460 core\nin vec3 pos;void main() {gl_Position = vec4(pos, 1.0);}";
    constexpr std::string_view ERROR_SHADER_FRAG = "#version 460 core\nout vec4 fragColor;void main() {fragColor = vec4(1.0, 0.0, 1.0, 1.0);}";

    std::optional<GLuint> compileShader(const std::string_view& source, GLenum shaderType)
    {
        int success;
        char infoLog[INFO_BUF_SIZE];

        const GLuint shaderId = glCreateShader(shaderType);

        const char* sourcePtr = source.data();
        const GLint srcLength = static_cast<GLint>(source.length());
        glShaderSource(shaderId, 1, &sourcePtr, &srcLength);

        glCompileShader(shaderId);

        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shaderId, INFO_BUF_SIZE, nullptr, infoLog);
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
            return { };
        }

        return compileShader(source.value(), shaderType);
    }

    ShaderProgramData makeShaderProgram(std::initializer_list<GLuint> shaderIds)
    {
        int success;
        char infoLog[INFO_BUF_SIZE];

        const GLuint programId = glCreateProgram();
        for (const GLuint shaderId: shaderIds)
        {
            glAttachShader(programId, shaderId);
        }
        glLinkProgram(programId);

        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programId, INFO_BUF_SIZE, nullptr, infoLog);
            spdlog::error("Shader linking failed:\n{}", infoLog);
        }

        for (const GLuint shaderId: shaderIds)
        {
            glDeleteShader(shaderId);
        }

        return ShaderProgramData{programId};
    }

    ShaderProgramData compileErrorShader()
    {
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

        return ShaderProgramData{pId};
    }
}

namespace
{
    struct DefaultResources
    {
        render::shader::ShaderProgramData errorShader;
    };

    DefaultResources _defaults;
}

void res::initResources()
{
    _defaults.errorShader = shader_intl::compileErrorShader();
}

void res::unloadResources() { }

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
        return shader_intl::makeShaderProgram({vert.value(), frag.value()});
    }
    return _defaults.errorShader;
}
