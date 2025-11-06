#pragma once

#include <glad/glad.h>

namespace graphics::shader
{
    struct ShaderProgramData
    {
        GLuint programId;
    };

    void use(ShaderProgramData data);
}
