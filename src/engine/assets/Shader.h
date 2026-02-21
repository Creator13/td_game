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

    using ShaderProperty = u64;

    struct PropertyInfo { };

    struct SamplerInfo { };

    struct UniformBlockInfo { };

    struct ShaderPipelineLayout
    {
        static ShaderPipelineLayout buildFromShader(gl::program_t program);
        usize uboSize() const;
    };

    struct Shader
    {
        explicit Shader(gl::program_t programId);
        ~Shader();

        const gl::program_t programId;

        static assets::AssetRef<Shader> fromFiles(std::string_view vertPath, std::string_view fragPath);
        static std::string createCombinedShaderPath(std::string_view vertPath, std::string_view fragPath);

        [[nodiscard]]
        const ShaderPipelineLayout& getLayout() const { return _layout; }

    private:
        ShaderPipelineLayout _layout;
    };
}
