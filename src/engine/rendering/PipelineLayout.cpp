#include "PipelineLayout.h"

#include <string>
#include <vector>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include "core/Assert.h"

using namespace core;
using namespace core::gfx;

namespace
{
    constexpr GLenum getGlSamplerTargetForUniformType(GLenum type)
    {
        switch (type)
        {
            case GL_SAMPLER_2D: return GL_TEXTURE_2D;
            case GL_SAMPLER_CUBE: return GL_TEXTURE_CUBE_MAP;
            case GL_SAMPLER_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
            case GL_SAMPLER_3D: return GL_TEXTURE_3D;
            case GL_SAMPLER_2D_SHADOW: return GL_TEXTURE_2D;
            default: return GL_TEXTURE_2D;
        }
    }

    constexpr bool isGlSampler(GLenum type)
    {
        return type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE ||
               type == GL_SAMPLER_2D_ARRAY || type == GL_SAMPLER_3D ||
               type == GL_SAMPLER_2D_SHADOW;
    }
}

ShaderPipelineLayout::ShaderPipelineLayout(gl::program_t program)
    : _program(program), _uniformBlocks(), _shaderProperties(8) { }

ShaderPipelineLayout ShaderPipelineLayout::buildFromShader(gl::program_t program)
{
    ShaderPipelineLayout resultLayout(program);

    // ## UBO reflection ##
    gl::Int numBlocks = 0;
    glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);

    // TODO quick and dirty safety check, if we need more blocks in a shader then either extned the array by default
    ENGINE_ASSERT(numBlocks <= MAX_BLOCKS, "Shader has more blocks than supported by pipeline, aborting... (program id: {})", program.id);

    constexpr gl::enum_t blockProperties[] = {GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH};
    constexpr gl::enum_t blockUniformProps[] = {GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE, GL_NAME_LENGTH};

    for (int iBlock = 0; iBlock < numBlocks; ++iBlock)
    {
        UniformBlockInfo& blockInfo = resultLayout._uniformBlocks[iBlock];
        blockInfo.index = iBlock; // ?? this seems unnecessary?

        gl::Int blockProps[4];
        glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, iBlock, 4, blockProperties, 4, nullptr, blockProps);
        blockInfo.binding = blockProps[0];
        blockInfo.dataSize = blockProps[1];

        const gl::Int numActiveVariables = blockProps[2];
        blockInfo.propertyCount = numActiveVariables;

        const gl::Int blockNameLength = blockProps[3];

        blockInfo.name.resize(blockNameLength);
        glGetProgramResourceName(program, GL_UNIFORM_BLOCK, iBlock, blockNameLength, nullptr, blockInfo.name.data());
        blockInfo.name.resize(blockNameLength - 1);

        // Sort block
        if (blockInfo.name == "MaterialBlock" || blockInfo.name == "Material")
        {
            ENGINE_ASSERT(resultLayout._materialBlockIndex == -1, "Found illegal second material block in program! (program id: {})", program.id);
            resultLayout._materialBlockIndex = iBlock;
        }
        else if (blockInfo.name == "FrameDataBlock" || blockInfo.name == "Frame" || blockInfo.name == "FrameData")
        {
            ENGINE_ASSERT(resultLayout._materialBlockIndex == -1, "Found illegal second frame data block in program! (program id: {})", program.id);
            resultLayout._frameDataBlockIndex = iBlock;
        }

        if (numActiveVariables == 0)
        {
            // Skip; Block contains 0 variables.
            continue;
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

        if (!isGlSampler(params[1]))
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

bool ShaderPipelineLayout::hasMaterialBlock() const
{
    return _materialBlockIndex > -1;
}

bool ShaderPipelineLayout::hasFrameDataBlock() const
{
    return _frameDataBlockIndex > -1;
}

const UniformBlockInfo& ShaderPipelineLayout::getMaterialBlockInfo() const
{
    ENGINE_ASSERT(hasMaterialBlock(), "Illegal call to get material block on shader without said block. (This block should not be missing if you see this message.)");
    return _uniformBlocks[_materialBlockIndex];
}

const UniformBlockInfo& ShaderPipelineLayout::getFrameDataBlockInfo() const
{
    ENGINE_ASSERT(hasMaterialBlock(), "Illegal call to get frame data block on shader without said block. (This block should not be missing if you see this message.)");
    return _uniformBlocks[_frameDataBlockIndex];
}
