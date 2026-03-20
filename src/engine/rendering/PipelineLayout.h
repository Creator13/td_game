#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <variant>
#include <fmt/format.h>

#include "datatype.h"
#include "rendering/ShaderPropertyId.h"

namespace core::gfx
{
    struct UniformBlockInfo
    {
        std::string name;
        gl::Uint index;
        gl::Int binding;
        int propertyCount;
        usize dataSize;
    };

    struct SamplerInfo
    {
        gl::Int location;
        gl::Int textureUnit;
        gl::enum_t target;
    };

    struct UniformInfo
    {
        gl::Int blockIndex;
        gl::Int offset;
        gl::Int size;
    };

    struct ShaderPropertyInfo
    {
        enum class PropertyType { Sampler, Uniform };

        std::string name;
        gl::enum_t glType;
        PropertyType propertyType;
        std::variant<UniformInfo, SamplerInfo> data;
    };

    struct ShaderPipelineLayout
    {
    private:
        constexpr static int MAX_BLOCKS = 4;

        int _numBlocks;
        int _materialBlockIndex = -1;
        int _frameDataBlockIndex = -1;

        gl::program_t _program;

        std::unordered_map<ShaderPropertyId, ShaderPropertyInfo> _shaderProperties;
        std::array<UniformBlockInfo, MAX_BLOCKS> _uniformBlocks;

        explicit ShaderPipelineLayout(gl::program_t program);

    public:
        static ShaderPipelineLayout buildFromShader(gl::program_t program);

        bool hasMaterialBlock() const;
        bool hasFrameDataBlock() const;

        const UniformBlockInfo& getMaterialBlockInfo() const;
        const UniformBlockInfo& getFrameDataBlockInfo() const;

        std::string toString() const;
    };
}

template<>
struct fmt::formatter<core::gfx::ShaderPipelineLayout> : formatter<std::string>
{
    auto format(const core::gfx::ShaderPipelineLayout& layout, format_context& ctx) const
    {
        return formatter<std::string>::format(layout.toString(), ctx);
    }
};