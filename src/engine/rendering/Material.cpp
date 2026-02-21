#include "Material.h"

#include <glad/gl.h>

#include "rendering/Pipeline.h"

using namespace core;
using namespace core::gfx;

Material::Material(const Pipeline& pipeline)
    : pipeline(pipeline), _ubo(0), _dirty(false), _layout(pipeline.getShaderLayout())
{
    usize uboSize = _layout.uboSize();
    if (uboSize > 0)
    {
        glCreateBuffers(1, &_ubo.id);
        glNamedBufferStorage(_ubo, uboSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }
}

Material::~Material()
{
    if (_ubo > 0)
    {
        glDeleteBuffers(1, &_ubo.id);
    }
}
