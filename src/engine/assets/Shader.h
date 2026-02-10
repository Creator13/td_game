#pragma once

#include <string_view>

#include "datatype.h"
#include "assets/AssetRef.h"

namespace core
{
    struct Shader;

    template<>
    struct assets::AssetTraits<Shader>
    {
        static constexpr AssetType type = AssetType::Shader;
    };

    struct Shader
    {
        explicit Shader(gl::program_t programId);
        ~Shader();

        const gl::program_t programId;

        // cached locations
        // etc

        static assets::AssetRef<Shader> fromFiles(std::string_view vertPath, std::string_view fragPath);
        static std::string createCombinedShaderPath(std::string_view vertPath, std::string_view fragPath);
    };
}
