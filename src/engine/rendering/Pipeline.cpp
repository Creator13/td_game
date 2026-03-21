#include "Pipeline.h"

#include <glad/gl.h>

#include "assets/AssetDatabase.h"
#include "assets/ShaderLoader.h"
#include "rendering/PipelineLayout.h"
#include "util/PagedStorage.h"

using namespace core;
using namespace core::gfx;
using namespace core::assets;

namespace
{
    ShaderLoader shaderLoader;

    util::PagedStorage<Pipeline, 64> pipelineStorage;
    util::PagedStorage<Material, 64> materialStorage;
}

PipelineDescriptor::PipelineDescriptor()
    : depthTest(true), depthFunc(GL_LESS),
      blend(false), blendSource(GL_SRC_ALPHA), blendDestination(GL_ONE_MINUS_SRC_ALPHA),
      backfaceCulling(BackfaceCulling::Back) { }

Pipeline::Pipeline(const PipelineDescriptor& descriptor, gl::program_t program)
    : _descriptor(descriptor), _programId(program), _shaderLayout(ShaderLayout::buildFromProgram(program)) { }

Pipeline::~Pipeline()
{
    glDeleteProgram(_programId);
}

AssetRef<Material> Pipeline::newMaterialInstance() const
{
    Material& mat = materialStorage.emplace(*this);
    return AssetRef(&mat, AssetId::idFromPath(fmt::format("Material {}", materialStorage.size())));
}

const ShaderLayout& Pipeline::getShaderLayout() const
{
    return _shaderLayout;
}

AssetRef<Pipeline> Pipeline::create(std::string_view name, const PipelineDescriptor& descriptor, std::string_view vertProgram, std::string_view fragProgram)
{
    const gl::program_t program = shaderLoader.glProgramFromFiles(vertProgram, fragProgram);

    Pipeline* pipeline = pipelineStorage.allocate_uninitialized();
    ::new(pipeline) Pipeline(descriptor, program);

    return AssetRef(pipeline, AssetId::idFromPath(fmt::format("@internal/pipeline/{}", name)));
}
