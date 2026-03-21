#pragma once

#include <string_view>

#include "datatype.h"
#include "assets/AssetRef.h"

namespace core
{
    // TODO This was leftover from when shaders were an official AssetType.
    //  Revisit if it is really necessary for what is essentially a wrapper class to still exist
    struct Shader
    {
        explicit Shader(gl::program_t programId);
        ~Shader();

        const gl::program_t programId;

        static Shader fromFiles(std::string_view vertPath, std::string_view fragPath);
        static std::string createCombinedShaderPath(std::string_view vertPath, std::string_view fragPath);
    };
}
