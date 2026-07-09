#include "Pipeline.h"

#include <glad/gl.h>

#include "assets/AssetDatabase.h"
#include "assets/ShaderLoader.h"
#include "rendering/ShaderLayout.h"
#include "util/PagedStorage.h"

using namespace core;
using namespace core::gfx;
using namespace core::assets;


// PipelineDescriptor::PipelineDescriptor()
//     : depthTest(true), depthFunc(DepthFunction::Less),
//       blend(false), blendSource(BlendOption::SourceAlpha), blendDestination(BlendOption::OneMinusSourceAlpha),
//       backfaceCulling(BackfaceCulling::Back) { }

Pipeline::Pipeline(const PipelineDescriptor& descriptor, gl::program_t program, u16 sortKey)
    : _descriptor(descriptor), _programId(program), _shaderLayout(ShaderLayout::buildFromProgram(program)),
      sortKey(sortKey)
{
    ENGINE_ASSERT(sortKey < 0xFFFF, "Sort key out of range (65535).");
}

Pipeline::~Pipeline()
{
    glDeleteProgram(_programId);
}

AssetRef<Material> Pipeline::newMaterialInstance(std::string_view name) const
{
    auto& materialStorage = AssetDatabase::instance->_materialStorage;
    auto [mem, index] = materialStorage.allocate_uninitialized();
    Material* mat = ::new(mem) Material(*this, index);

    return AssetDatabase::registerRuntimeAsset<Material>(name, mat, index);
}

const ShaderLayout& Pipeline::getShaderLayout() const
{
    return _shaderLayout;
}

AssetRef<Pipeline> Pipeline::create(std::string_view name, const PipelineDescriptor& descriptor, std::string_view vertSourcePath, std::string_view fragSourcePath)
{
    const gl::program_t program = AssetDatabase::instance->_shaderLoader.glProgramFromFiles(vertSourcePath, fragSourcePath);

    auto& pipelineStorage = AssetDatabase::instance->_pipelineStorage;
    auto [mem, index] = pipelineStorage.allocate_uninitialized();
    Pipeline* pipeline = ::new(mem) Pipeline(descriptor, program, index);

    return AssetDatabase::registerRuntimeAsset<Pipeline>(name, pipeline, index);
}

AssetRef<Pipeline> Pipeline::createFullscreenEffect(std::string_view name, std::string_view fullscreenShaderSourcePath)
{
    constexpr PipelineDescriptor fsDescriptor{
        .depthTest = false,
        .blend = false,
        .backfaceCulling = BackfaceCulling::Back,
    };

    return create(name, fsDescriptor, "shaders/fullscreen.vert", fullscreenShaderSourcePath);
}

gl::enum_t gl_platform::getGlBlendFuncOption(BlendOption in)
{
    switch (in)
    {
        case BlendOption::Zero:
            return GL_ZERO;
        case BlendOption::One:
            return GL_ONE;
        case BlendOption::SourceColor:
            return GL_SRC_COLOR;
        case BlendOption::OneMinusSourceColor:
            return GL_ONE_MINUS_SRC_COLOR;
        case BlendOption::SourceAlpha:
            return GL_SRC_ALPHA;
        case BlendOption::OneMinusSourceAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BlendOption::DestAlpha:
            return GL_DST_ALPHA;
        case BlendOption::OneMinusDestAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
        default:
            ENGINE_ASSERT(false, "Invalid blending option: {}", magic_enum::enum_name(in));
            ENGINE_UNREACHABLE();
    }
}

gl::enum_t gl_platform::getGlDepthFunc(DepthFunction in)
{
    switch (in)
    {
        case DepthFunction::Always:
            return GL_ALWAYS;
        case DepthFunction::Never:
            return GL_NEVER;
        case DepthFunction::Less:
            return GL_LESS;
        case DepthFunction::Equal:
            return GL_EQUAL;
        case DepthFunction::LesserEqual:
            return GL_LEQUAL;
        case DepthFunction::Greater:
            return GL_GREATER;
        case DepthFunction::GreaterEqual:
            return GL_GEQUAL;
        case DepthFunction::NotEqual:
            return GL_NOTEQUAL;
        default:
            ENGINE_ASSERT(false, "Invalid depth function: {}", magic_enum::enum_name(in));
            ENGINE_UNREACHABLE();
    }
}
