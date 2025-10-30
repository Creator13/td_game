#include "resources.h"

#include <fstream>
#include <spdlog/spdlog.h>

constexpr uint32_t INFO_BUF_SIZE = 512;

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

namespace shader_intl
{
    std::optional<GLuint> compileShader(const fs::path& path, GLenum shaderType)
    {
        int success;
        char infoLog[INFO_BUF_SIZE];

        const GLuint shaderId = glCreateShader(shaderType);

        std::optional<std::string> source = res::readFileText(path);
        if (!source.has_value())
        {
            return { };
        }

        const char* sourcePtr = source.value().c_str();
        const GLint srcLength = static_cast<GLint>(source.value().length());
        glShaderSource(shaderId, 1, &sourcePtr, &srcLength);

        glCompileShader(shaderId);

        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shaderId, INFO_BUF_SIZE, nullptr, infoLog);
            spdlog::error("Shader compilation failed:\n{}", infoLog);
            return { };
        }

        return shaderId;
    }

    render::shader::ShaderProgramData makeShaderProgram(std::initializer_list<GLuint> shaderIds)
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

        return render::shader::ShaderProgramData{programId};
    }
}

render::shader::ShaderProgramData res::loadShader(const fs::path& vertPath, const fs::path& fragPath)
{
    std::optional<GLuint> vert = shader_intl::compileShader(vertPath, GL_VERTEX_SHADER);
    std::optional<GLuint> frag = shader_intl::compileShader(fragPath, GL_FRAGMENT_SHADER);

    if (vert && frag)
    {
        return shader_intl::makeShaderProgram({vert.value(), frag.value()});
    }
    return shader::ShaderProgramData{0};
}
