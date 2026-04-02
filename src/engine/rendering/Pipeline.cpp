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

Pipeline::Pipeline(const PipelineDescriptor& descriptor, gl::program_t program, u16 sortKey)
    : sortKey(sortKey), _descriptor(descriptor), _programId(program),
      _shaderLayout(ShaderLayout::buildFromProgram(program))
{
    ENGINE_ASSERT(sortKey < 0xFFFF, "Sort key out of range (65535).");
}

Pipeline::~Pipeline()
{
    glDeleteProgram(_programId);
}

AssetRef<Material> Pipeline::newMaterialInstance() const
{
    void* mem = materialStorage.allocate_uninitialized();
    Material* mat = ::new(mem) Material(*this, materialStorage.size() - 1);

    return AssetRef(mat, AssetId::idFromPath(fmt::format("@internal/material/{}", materialStorage.size() - 1)));
}

const ShaderLayout& Pipeline::getShaderLayout() const
{
    return _shaderLayout;
}

AssetRef<Pipeline> Pipeline::create(std::string_view name, const PipelineDescriptor& descriptor, std::string_view vertProgram, std::string_view fragProgram)
{
    const gl::program_t program = shaderLoader.glProgramFromFiles(vertProgram, fragProgram);

    void* mem = pipelineStorage.allocate_uninitialized();
    Pipeline* pipeline = ::new(mem) Pipeline(descriptor, program, pipelineStorage.size() - 1);

    return AssetRef(pipeline, AssetId::idFromPath(fmt::format("@internal/pipeline/{}", name)));
}
