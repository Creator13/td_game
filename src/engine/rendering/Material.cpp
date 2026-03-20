#include "Material.h"

#include <glad/gl.h>

#include "rendering/Pipeline.h"

using namespace core;
using namespace core::gfx;
using namespace math;

Material::Material(const Pipeline& pipeline)
    : pipeline(pipeline), _layout(pipeline.getShaderLayout()), _uboHandle(0), _dirty(false)
{
    const usize uboSize = _layout.getMaterialBlockInfo().dataSize;

    if (uboSize > 0)
    {
        glCreateBuffers(1, &_uboHandle.id);
        glNamedBufferStorage(_uboHandle, uboSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    _materialBlockDdata.resize(uboSize);
}

Material::~Material()
{
    if (_uboHandle > 0)
    {
        glDeleteBuffers(1, &_uboHandle.id);
    }
}

void Material::setFloat(ShaderPropertyId id, float value) { }

void Material::setInt(ShaderPropertyId id, int value) { }

void Material::setVec2(ShaderPropertyId id, vec2 value) { }

void Material::setVec3(ShaderPropertyId id, vec3 value) { }

void Material::setVec4(ShaderPropertyId id, vec4 value) { }

void Material::setColor(ShaderPropertyId id, Color value)
{
    setVec4(id, static_cast<vec4>(value));
}

void Material::setMat4(ShaderPropertyId id, const mat4& value) { }

void Material::setTexture2D(ShaderPropertyId id, assets::AssetRef<Texture> tex) { }
