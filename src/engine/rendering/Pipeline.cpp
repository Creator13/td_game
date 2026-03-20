#include "Pipeline.h"

#include "glad/gl.h"

using namespace core::gfx;

PipelineDescriptor::PipelineDescriptor()
    : depthTest(true), depthFunc(GL_LESS),
      blend(false), blendSource(GL_SRC_ALPHA), blendDestination(GL_ONE_MINUS_SRC_ALPHA),
      backfaceCulling(BackfaceCulling::Back) { }

Pipeline::Pipeline(const PipelineDescriptor& descriptor, assets::AssetRef<Shader> shader)
    : _descriptor(descriptor), _shader(shader) { }

core::Material Pipeline::newMaterialInstance() const
{
    return Material(*this);
}

const ShaderPipelineLayout& Pipeline::getShaderLayout() const
{
    return _shader->getLayout();
}
