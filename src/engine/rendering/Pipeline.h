#pragma once

#include "datatype.h"
#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "rendering/ShaderLayout.h"

namespace core::gfx
{
    class Renderer;

    enum class DepthFunction : u8
    {
        Always, Never, Less, Equal, LesserEqual, Greater, GreaterEqual, NotEqual
    };

    enum class BlendOption : u8
    {
        Zero, One,
        SourceColor, OneMinusSourceColor,
        SourceAlpha, OneMinusSourceAlpha,
        DestAlpha, OneMinusDestAlpha
    };

    enum class BackfaceCulling : u8
    {
        Back, Front, None
    };

    struct PipelineDescriptor
    {
        // TODO make constructors that automatically set bools

        bool depthTest = true;
        DepthFunction depthFunc = DepthFunction::Less;

        bool blend = false;
        BlendOption blendSource = BlendOption::One;
        BlendOption blendDestination = BlendOption::Zero;

        BackfaceCulling backfaceCulling = BackfaceCulling::Back;
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

        static assets::AssetRef<Pipeline> create(std::string_view name, const PipelineDescriptor& descriptor, std::string_view vertSourcePath, std::string_view fragSourcePath);
        static assets::AssetRef<Pipeline> createFullscreenEffect(std::string_view name, std::string_view fullscreenShaderSourcePath);
    };

    namespace gl_platform
    {
        gl::enum_t getGlBlendFuncOption(BlendOption in);
        gl::enum_t getGlDepthFunc(DepthFunction in);
    }
}
