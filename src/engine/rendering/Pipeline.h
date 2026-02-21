#pragma once

#include "datatype.h"
#include "assets/Shader.h"
#include "rendering/Material.h"

namespace core::gfx
{
    class Renderer;

    enum class BackfaceCulling : u8 { Back, Front, None };

    struct PipelineDescriptor
    {
        PipelineDescriptor();

        bool depthTest;
        gl::enum_t depthFunc;

        bool blend = false;
        gl::enum_t blendSource;
        gl::enum_t blendDestination;

        BackfaceCulling backfaceCulling;
    };

    class Pipeline
    {
        friend Renderer;

        PipelineDescriptor _descriptor;
        assets::AssetRef<Shader> _shader;

    public:
        Pipeline(const PipelineDescriptor& descriptor, assets::AssetRef<Shader> shader);

        Material newMaterialInstance() const;
        const ShaderPipelineLayout& getShaderLayout() const;
    };
}
