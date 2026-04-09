#pragma once

#include "datatype.h"
#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "rendering/ShaderLayout.h"

namespace core::gfx
{
    class Renderer;

    enum class BackfaceCulling : u8 { Back, Front, None };

    struct PipelineDescriptor
    {
        PipelineDescriptor();

        bool depthTest;
        gl::enum_t depthFunc;

        bool blend;
        gl::enum_t blendSource;
        gl::enum_t blendDestination;

        BackfaceCulling backfaceCulling;
    };

    class Pipeline
    {
        friend Renderer;

        PipelineDescriptor _descriptor;
        gl::program_t _programId;
        ShaderLayout _shaderLayout;

        Pipeline(const PipelineDescriptor& descriptor, gl::program_t program, u16 sortKey);

    public:
        const u16 sortKey;

        ~Pipeline();

        assets::AssetRef<Material> newMaterialInstance(std::string_view name) const;
        const ShaderLayout& getShaderLayout() const;

        static assets::AssetRef<Pipeline> create(std::string_view name, const PipelineDescriptor& descriptor, std::string_view vertProgram, std::string_view fragProgram);
    };
}
