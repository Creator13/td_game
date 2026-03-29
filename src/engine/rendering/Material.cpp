#include "Material.h"

#include <glad/gl.h>

#include "math/mat4.h"
#include "rendering/Pipeline.h"

using namespace core;
using namespace core::gfx;
using namespace math;

Material::Material(const Pipeline& pipeline)
    : _pipeline(pipeline), _layout(pipeline.getShaderLayout()), _uboHandle(0), _dirty(true)
{
    constructBuffers();
}

void Material::constructBuffers()
{
    ENGINE_ASSERT(_uboHandle == 0, "Material buffer construction called on material with buffer handle != 0, should only be called on material with uninitialized buffers.");

    if (!_layout.hasMaterialBlock()) return;

    if (const usize uboSize = _layout.getMaterialBlockInfo().dataSize; uboSize > 0)
    {
        glCreateBuffers(1, &_uboHandle.id);
        glNamedBufferStorage(_uboHandle, uboSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
        _materialBlockData.resize(uboSize);
    }
}

void Material::flushUboChangesToGpu()
{
    if (!_layout.hasMaterialBlock()) return;
    if (!_dirty) return;

    ENGINE_ASSERT(_uboHandle > 0, "Material buffer was non initialized.");

    // Set data
    glNamedBufferSubData(_uboHandle, 0, _layout.getMaterialBlockInfo().dataSize, _materialBlockData.data());
    _dirty = false;
}

Material::~Material()
{
    if (_uboHandle > 0)
    {
        glDeleteBuffers(1, &_uboHandle.id);
        _uboHandle = 0;
    }
}

void Material::setFloat(ShaderPropertyId id, float value)
{
    setUniform<float>(id, value, GL_FLOAT);
}

void Material::setInt(ShaderPropertyId id, int value)
{
    setUniform<int>(id, value, GL_INT);
}

void Material::setVec2(ShaderPropertyId id, vec2 value)
{
    setUniform<vec2>(id, value, GL_FLOAT_VEC2);
}

void Material::setVec3(ShaderPropertyId id, vec3 value)
{
    setUniform<vec3>(id, value, GL_FLOAT_VEC3);
}

void Material::setVec4(ShaderPropertyId id, vec4 value)
{
    setUniform<vec4>(id, value, GL_FLOAT_VEC4);
}

void Material::setColor(ShaderPropertyId id, Color value)
{
    // Color is a vec4 in shaders; alias function
    setVec4(id, static_cast<vec4>(value));
}

void Material::setMat4(ShaderPropertyId id, const mat4& value)
{
    setUniform<mat4>(id, value, GL_FLOAT_MAT4);
}

void Material::setTexture2D(ShaderPropertyId id, assets::AssetRef<Texture> tex)
{
    const ShaderPropertyInfo* const property = _layout.getPropertyInfo(id);
    if (!property || property->propertyType != ShaderPropertyInfo::PropertyType::Sampler)
    {
        // Property does not exist; bounce.
        return;
    }

    _textures[id] = tex;
}
