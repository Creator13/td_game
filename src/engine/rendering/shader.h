#pragma once
#include <cstdint>

namespace graphics
{
    struct ShaderProgramData
    {
        uint32_t programId;
    };

    namespace shader
    {
        void use(ShaderProgramData data);
    }
}
