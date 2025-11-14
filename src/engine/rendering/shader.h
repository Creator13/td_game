#pragma once
#include <cstdint>

namespace graphics::shader
{
    struct ShaderProgramData
    {
        uint32_t programId;
    };

    void use(ShaderProgramData data);
}
