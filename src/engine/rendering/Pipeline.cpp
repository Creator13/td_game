#include "Pipeline.h"

#include <glad/gl.h>

#include "assets/AssetDatabase.h"
#include "assets/ShaderLoader.h"
#include "rendering/PipelineLayout.h"
#include "util/PagedStorage.h"

using namespace core;
using namespace core::gfx;
using namespace core::assets;


PipelineDescriptor::PipelineDescriptor()
    : depthTest(true), depthFunc(GL_LESS),
      blend(false), blendSource(GL_SRC_ALPHA), blendDestination(GL_ONE_MINUS_SRC_ALPHA),
      backfaceCulling(BackfaceCulling::Back) { }

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
    void* mem = materialStorage.allocate_uninitialized();
    Material* mat = ::new(mem) Material(*this, materialStorage.size() - 1);

    return AssetDatabase::registerRuntimeAsset<Material>(name, mat);
}

const ShaderLayout& Pipeline::getShaderLayout() const
{
    return _shaderLayout;
}

AssetRef<Pipeline> Pipeline::create(std::string_view name, const PipelineDescriptor& descriptor, std::string_view vertProgram, std::string_view fragProgram)
{
    const gl::program_t program = AssetDatabase::instance->_shaderLoader.glProgramFromFiles(vertProgram, fragProgram);

    auto& pipelineStorage = AssetDatabase::instance->_pipelineStorage;
    void* mem = pipelineStorage.allocate_uninitialized();
    Pipeline* pipeline = ::new(mem) Pipeline(descriptor, program, pipelineStorage.size() - 1);

    return AssetDatabase::registerRuntimeAsset<Pipeline>(name, pipeline);
}
