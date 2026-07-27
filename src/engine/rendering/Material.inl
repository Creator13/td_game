#pragma once

template<typename T>
void Material::setUniform(gfx::ShaderPropertyId id, const T& value, gl::enum_t expectedGlType)
{
    const gfx::ShaderPropertyInfo* const property = _layout.getPropertyInfo(id);
    if (!property || property->propertyType != gfx::ShaderPropertyInfo::PropertyType::Uniform)
    {
        // Property does not exist; bounce
        return;
    }

    if (property->glType != expectedGlType)
    {
        // Bounce for wrong type
        SPDLOG_WARN("Shader uniform type mismatch trying to set property id {}. Expected type {}, was {}.", id, expectedGlType, property->glType);
        return;
    }

    const gfx::UniformInfo& uniformInfo = property->getUniformInfo();
    if (uniformInfo.blockIndex != _layout.getMaterialBlockIndex())
    {
        // Tried to access non-material property, which is disallowed. Just bounce (but TODO maybe warn?)
        return;
    }

    std::memcpy(_materialBlockData.data() + uniformInfo.offset, &value, sizeof(T));
    _dirty = true;
}