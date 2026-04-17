#include "Material.h"

#include <glad/gl.h>

#include "formatting/fmt_gl.h"
#include "math/mat4.h"
#include "rendering/Pipeline.h"

using namespace core;
using namespace core::gfx;
using namespace math;

Material::Material(const Pipeline& pipeline, u16 sortKey)
    : _layout(pipeline.getShaderLayout()), _uboHandle(0), _dirty(true), pipeline(pipeline), sortKey(sortKey)
{
    ENGINE_ASSERT(sortKey < 0xFFFF, "Sort key out of range (65535).");
    constructBuffers();
    initializeData();
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

    _textures.reserve(_layout.getSamplerCount());
}

void Material::initializeData()
{
    // Init all data to zero (probably unnecessary considering constructBuffers does a resize, aka zero-init)
    std::memset(_materialBlockData.data(), 0, _materialBlockData.size());

    for (auto& [id, property] : _layout.properties())
    {
        // Assign null ref to every texture unit; rely on renderer to know how to deal with null refs.
        if (property.propertyType == ShaderPropertyInfo::PropertyType::Sampler)
        {
            _textures.insert_or_assign(id, assets::AssetRef<Texture>::null());
        }

        // Most uniforms will have {0} as their default value
        if (property.propertyType == ShaderPropertyInfo::PropertyType::Uniform)
        {
            // Exception is mat4s, better to set those to identity.
            if (property.glType == GL_FLOAT_MAT4)
            {
                setMat4(id, mat4::identity);
            }
        }
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
    if (property == nullptr || property->propertyType != ShaderPropertyInfo::PropertyType::Sampler)
    {
        // Property does not exist; bounce.
        return;
    }

    _textures.insert_or_assign(id, tex);
}

void Material::setBuffer(ShaderPropertyId id, const GraphicsBuffer* buffer)
{
    const ShaderPropertyInfo* const property = _layout.getPropertyInfo(id);
    if (property == nullptr || property->propertyType != ShaderPropertyInfo::PropertyType::Buffer)
    {
        // Property does not exist; bounce.
        return;
    }

    _buffers.insert_or_assign(id, buffer);
}
