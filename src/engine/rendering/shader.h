#pragma once

#include <complex.h>
#include <glad/glad.h>

namespace render::shader
{
    struct ShaderProgramData
    {
        GLuint programId;
    };

    void use(ShaderProgramData data);
}
