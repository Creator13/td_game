#pragma once

#include <array>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <variant>
#include <fmt/format.h>

#include "datatype.h"
#include "rendering/ShaderPropertyId.h"

namespace core
{
    class Material;
}

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

        const UniformInfo& getUniformInfo() const;
        const SamplerInfo& getSamplerInfo() const;
    };

    struct ShaderLayout
    {
    private:
        constexpr static int MAX_UBOS = 4;

        int _numUBOs;
        int _numSamplers;

        int _materialBlockIndex = -1;
        int _frameDataBlockIndex = -1;

        gl::program_t _program;

        std::unordered_map<ShaderPropertyId, ShaderPropertyInfo> _shaderProperties;
        std::array<UniformBlockInfo, MAX_UBOS> _uniformBlocks;

        explicit ShaderLayout(gl::program_t program);

    public:
        static ShaderLayout buildFromProgram(gl::program_t program);

        bool hasMaterialBlock() const;
        bool hasFrameDataBlock() const;
        int getSamplerCount() const { return _numSamplers; }

        int getMaterialBlockIndex() const { return _materialBlockIndex; }
        const UniformBlockInfo& getMaterialBlockInfo() const;

        int getFrameDataBlockIndex() const { return _frameDataBlockIndex; }
        const UniformBlockInfo& getFrameDataBlockInfo() const;

        const ShaderPropertyInfo* getPropertyInfo(ShaderPropertyId id) const;
        auto properties() const { return std::views::all(_shaderProperties); }

        std::string toString() const;
    };
}

template<>
struct fmt::formatter<core::gfx::ShaderPropertyInfo> : formatter<std::string_view>
{
    auto format(const core::gfx::ShaderPropertyInfo& propInfo, format_context& ctx) const
    {
        return formatter<std::string_view>::format(
            fmt::format("{} (id:{})", propInfo.name, core::gfx::makePropertyId(propInfo.name)), ctx);
    }
};

template<>
struct fmt::formatter<core::gfx::ShaderLayout> : formatter<std::string>
{
    auto format(const core::gfx::ShaderLayout& layout, format_context& ctx) const
    {
        return formatter<std::string>::format(layout.toString(), ctx);
    }
};
