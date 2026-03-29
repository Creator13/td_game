#include "PipelineLayout.h"

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include "core/Assert.h"
#include "formatting/fmt_gl.h"

using namespace core;
using namespace core::gfx;

namespace
{
    constexpr GLenum getGlSamplerTargetForUniformType(GLenum type)
    {
        switch (type)
        {
            case GL_SAMPLER_2D:
                return GL_TEXTURE_2D;
            case GL_SAMPLER_CUBE:
                return GL_TEXTURE_CUBE_MAP;
            case GL_SAMPLER_2D_ARRAY:
                return GL_TEXTURE_2D_ARRAY;
            case GL_SAMPLER_3D:
                return GL_TEXTURE_3D;
            case GL_SAMPLER_2D_SHADOW:
                return GL_TEXTURE_2D;
            default:
                ENGINE_ASSERT(false, "please just don't call this function on non-sampler types?");
        }
    }

    constexpr bool glTypeIsSampler(GLenum type)
    {
        return type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE ||
               type == GL_SAMPLER_2D_ARRAY || type == GL_SAMPLER_3D ||
               type == GL_SAMPLER_2D_SHADOW;
    }
}

const UniformInfo& ShaderPropertyInfo::getUniformInfo() const
{
    ENGINE_ASSERT(propertyType == PropertyType::Uniform, "Cannot get uniform info on a sampler property.");
    return std::get<UniformInfo>(data);
}

const SamplerInfo& ShaderPropertyInfo::getSamplerInfo() const
{
    ENGINE_ASSERT(propertyType == PropertyType::Sampler, "Cannot get sampler info on a uniform property.");
    return std::get<SamplerInfo>(data);
}

ShaderLayout::ShaderLayout(gl::program_t program)
    : _numBlocks(0), _program(program), _shaderProperties(8), _uniformBlocks() { }

ShaderLayout ShaderLayout::buildFromProgram(gl::program_t program)
{
    ShaderLayout resultLayout(program);

    // ## UBO reflection ##
    gl::Int numBlocks = 0;
    glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);

    resultLayout._numBlocks = numBlocks;

    // TODO quick and dirty safety check, if we need more blocks in a shader then either extend the array by default or make it a vector
    ENGINE_ASSERT(numBlocks <= MAX_BLOCKS, "Shader has more blocks than supported by pipeline, aborting... (program id: {})", program);

    constexpr gl::enum_t blockProperties[] = {GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH};
    constexpr gl::enum_t blockUniformProps[] = {GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE, GL_NAME_LENGTH};

    for (int iBlock = 0; iBlock < numBlocks; ++iBlock)
    {
        UniformBlockInfo& blockInfo = resultLayout._uniformBlocks[iBlock];
        blockInfo.index = iBlock;

        gl::Int blockProps[4];
        glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, iBlock, 4, blockProperties, 4, nullptr, blockProps);

        const gl::Int numActiveVariables = blockProps[2];
        if (numActiveVariables == 0)
        {
            // Skip; Block contains 0 variables.
            continue;
        }

        blockInfo.propertyCount = numActiveVariables;
        blockInfo.binding = blockProps[0];
        blockInfo.dataSize = blockProps[1];

        const gl::Int blockNameLength = blockProps[3];

        blockInfo.name.resize(blockNameLength);
        glGetProgramResourceName(program, GL_UNIFORM_BLOCK, iBlock, blockNameLength, nullptr, blockInfo.name.data());
        blockInfo.name.resize(blockNameLength - 1);

        // Sort block
        if (blockInfo.name == "MaterialBlock" || blockInfo.name == "Material")
        {
            ENGINE_ASSERT(resultLayout._materialBlockIndex == -1, "Found illegal second material block in program! (program id: {})", program);
            resultLayout._materialBlockIndex = iBlock;
        }
        else if (blockInfo.name == "FrameDataBlock" || blockInfo.name == "Frame" || blockInfo.name == "FrameData")
        {
            ENGINE_ASSERT(resultLayout._materialBlockIndex == -1, "Found illegal second frame data block in program! (program id: {})", program);
            resultLayout._frameDataBlockIndex = iBlock;
        }

        std::vector<gl::Int> activeVariables(numActiveVariables);
        constexpr gl::enum_t activeVarProperty = GL_ACTIVE_VARIABLES;
        glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, iBlock, 1, &activeVarProperty, numActiveVariables, nullptr, activeVariables.data());

        for (gl::Int iBlockVariable = 0; iBlockVariable < numActiveVariables; iBlockVariable++)
        {
            gl::Int varIndex = activeVariables[iBlockVariable];
            ShaderPropertyInfo shaderPropInfo;
            shaderPropInfo.propertyType = ShaderPropertyInfo::PropertyType::Uniform;

            gl::Int varProps[4];
            glGetProgramResourceiv(program, GL_UNIFORM, varIndex, 4, blockUniformProps, 4, nullptr, varProps);

            shaderPropInfo.glType = varProps[0];

            UniformInfo uniformInfo;
            uniformInfo.offset = varProps[1];
            uniformInfo.size = varProps[2];
            uniformInfo.blockIndex = iBlock;
            shaderPropInfo.data = uniformInfo;

            const gl::Int varNameLength = varProps[3];
            shaderPropInfo.name.resize(varNameLength);
            glGetProgramResourceName(program, GL_UNIFORM, varIndex, varNameLength, nullptr, shaderPropInfo.name.data());
            shaderPropInfo.name.resize(varNameLength - 1);

            // sanitize for arrays (remove '[0]')
            if (shaderPropInfo.name.back() == ']')
            {
                shaderPropInfo.name = shaderPropInfo.name.substr(0, shaderPropInfo.name.find_first_of('['));
            }

            ShaderPropertyId spid = makePropertyId(shaderPropInfo.name);

            resultLayout._shaderProperties[spid] = shaderPropInfo;
        }
    }

    // ## Sampler reflection ##
    gl::Int numUniforms = 0;
    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    constexpr gl::enum_t uniformProps[] = {GL_BLOCK_INDEX, GL_TYPE, GL_LOCATION, GL_NAME_LENGTH};

    gl::Int textureUnit = 0;
    for (int iUniform = 0; iUniform < numUniforms; iUniform++)
    {
        gl::Int params[4];
        glGetProgramResourceiv(program, GL_UNIFORM, iUniform, 4, uniformProps, 4, nullptr, params);

        if (params[0] != -1)
        {
            // Skip; Already visited when reflection blocks.
            continue;
        }

        if (!glTypeIsSampler(params[1]))
        {
            // Not a sampler, ignore. TODO revisit this? at least set up some conventions for what kind of loose uniforms are allowed.
            continue;
        }

        ShaderPropertyInfo shaderPropInfo;
        shaderPropInfo.propertyType = ShaderPropertyInfo::PropertyType::Sampler;
        shaderPropInfo.glType = params[1];

        SamplerInfo samplerInfo;
        samplerInfo.location = params[2];
        samplerInfo.textureUnit = textureUnit++;
        samplerInfo.target = getGlSamplerTargetForUniformType(params[1]);
        shaderPropInfo.data = samplerInfo;

        glProgramUniform1i(program, samplerInfo.location, samplerInfo.textureUnit);

        const gl::Int uniformNameLength = params[3];
        shaderPropInfo.name.resize(uniformNameLength);
        glGetProgramResourceName(program, GL_UNIFORM, iUniform, uniformNameLength, nullptr, shaderPropInfo.name.data());
        shaderPropInfo.name.resize(uniformNameLength - 1);

        ShaderPropertyId spid = makePropertyId(shaderPropInfo.name);
        resultLayout._shaderProperties[spid] = shaderPropInfo;
    }

    return resultLayout;
}

bool ShaderLayout::hasMaterialBlock() const
{
    return _materialBlockIndex > -1;
}

bool ShaderLayout::hasFrameDataBlock() const
{
    return _frameDataBlockIndex > -1;
}

const UniformBlockInfo& ShaderLayout::getMaterialBlockInfo() const
{
    ENGINE_ASSERT(hasMaterialBlock(), "Illegal call to get material block on shader without said block. (This block should not be missing if you see this message.)");
    return _uniformBlocks[_materialBlockIndex];
}

const UniformBlockInfo& ShaderLayout::getFrameDataBlockInfo() const
{
    ENGINE_ASSERT(hasMaterialBlock(), "Illegal call to get frame data block on shader without said block. (This block should not be missing if you see this message.)");
    return _uniformBlocks[_frameDataBlockIndex];
}

const ShaderPropertyInfo* ShaderLayout::getPropertyInfo(ShaderPropertyId id) const
{
    const auto it = _shaderProperties.find(id);
    if (it == _shaderProperties.end())
    {
        spdlog::debug("Shader property id {} does not exist in shader (program id {}). This is likely fine, but I'm warning you nonetheless.", id, _program);
        return nullptr;
    }

    return &it->second;
}

std::string ShaderLayout::toString() const
{
    using IdPropertyPair = std::pair<ShaderPropertyId, ShaderPropertyInfo>;

    fmt::memory_buffer buffer;

    fmt::format_to(std::back_inserter(buffer), "\nShaderPipelineLayout (program id {}):\n", _program);

    for (int i = 0; i < _numBlocks; i++)
    {
        const UniformBlockInfo& block = _uniformBlocks[i];

        fmt::format_to(std::back_inserter(buffer),
            "## {} (index={}, binding={}, propertyCount={}, size={}b):\n",
            block.name, block.index, block.binding, block.propertyCount, block.dataSize);

        std::vector<IdPropertyPair> blockProperties;
        blockProperties.reserve(block.propertyCount);
        for (const auto& [id, property] : _shaderProperties)
        {
            if (property.propertyType != ShaderPropertyInfo::PropertyType::Uniform) continue;

            const auto& uniformInfo = std::get<UniformInfo>(property.data);
            if (uniformInfo.blockIndex == block.index)
            {
                blockProperties.push_back({id, property});
            }
        }

        // Sort by offset
        std::ranges::sort(blockProperties, [](const IdPropertyPair& a, const IdPropertyPair& b)
        {
            return std::get<UniformInfo>(a.second.data).offset < std::get<UniformInfo>(b.second.data).offset;
        });

        for (const auto& [spid, propInfo] : blockProperties)
        {
            const auto& uniformInfo = std::get<UniformInfo>(propInfo.data);
            fmt::format_to(std::back_inserter(buffer),
                "   |- {} (spid={}): {}, offset={}, arraySize={}\n",
                propInfo.name, spid, glTypeToString(propInfo.glType), uniformInfo.offset, uniformInfo.size);
        }
    }

    fmt::format_to(std::back_inserter(buffer), "## Samplers:\n");

    for (const auto& [id, property] : _shaderProperties)
    {
        if (property.propertyType != ShaderPropertyInfo::PropertyType::Sampler) continue;

        const auto& samplerInfo = std::get<SamplerInfo>(property.data);
        fmt::format_to(std::back_inserter(buffer),
            "   |- {} (spid={}): {}, location={}, texUnit={}",
            property.name, id, glTypeToString(property.glType), samplerInfo.location, samplerInfo.textureUnit);
    }

    return fmt::to_string(buffer);
}
