#include "Renderer.h"

#include <glad/gl.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "assets/Mesh.h"
#include "core/Constants.h"
#include "core/Time.h"
#include "rendering/DataLayout.h"
#include "rendering/Material.h"
#include "rendering/Pipeline.h"

using namespace math;
using namespace core;
using namespace core::gfx;

mat4 ViewportData::getCombinedViewProjectionMatrix() const
{
    return projectionMatrix * constants::COORDINATE_BASIS * viewMatrix;
}

mat4 ViewportData::getScreenSpaceProjectionMatrix() const
{
    return mat4::makeOrtho(0, pixelWidth, pixelHeight, 0, -1, 1) * constants::COORDINATE_BASIS;
}

// In ViewportData, or as free functions taking a ViewportData

// depth: 0.0 = on near plane, 1.0 = on far plane
vec3 ViewportData::screenToWorld(vec2 pixelPos, float depth) const
{
    // 1. pixel → NDC (OpenGL: y flipped, z remapped to [-1, 1])
    const float ndcX = (pixelPos.x / pixelWidth) * 2.f - 1.f;
    const float ndcY = -(pixelPos.y / pixelHeight) * 2.f + 1.f; // flip Y
    const float ndcZ = depth * 2.f - 1.f;

    // 2. unproject through inverse VP
    const mat4 vpInv = inverse(getCombinedViewProjectionMatrix());
    const vec4 clip = {ndcX, ndcY, ndcZ, 1.f};
    const vec4 world = vpInv * clip;

    // 3. CRITICAL: perspective divide
    return vec3{world.x, world.y, world.z} / world.w;
}

vec2 ViewportData::worldToScreen(vec3 worldPos) const
{
    // 1. world → clip space
    const vec4 clip = getCombinedViewProjectionMatrix() * vec4{worldPos, 1.f};

    // 2. perspective divide → NDC
    const float ndcX = clip.x / clip.w;
    const float ndcY = clip.y / clip.w;

    // 3. NDC → pixel (y flipped back)
    return {
        (ndcX + 1.f) * 0.5f * pixelWidth,
        (1.f - ndcY) * 0.5f * pixelHeight
    };
}

Renderer::Renderer()
    : _instanceDataBuffer(1_MB)
{
    glFrontFace(GL_CCW);
    glEnable(GL_FRAMEBUFFER_SRGB); // Set *default* framebuffer to convert back to srgb on present

    glCreateBuffers(1, &_frameDataUboHandle.id);
    glNamedBufferStorage(_frameDataUboHandle, sizeof(PassDataBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);


    glCreateSamplers(1, &_defaultSampler);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Renderer::setViewportData(const ViewportData& params)
{
    _viewportData = params;
}

void Renderer::submitDrawCommand(const DrawCommand& command)
{
    std::vector<DrawCommand>* targetQueue = nullptr;

    switch (command.queue)
    {
        case DrawCommand::RenderQueue::OPAQUE:
            targetQueue = &_opaqueCommandQueue;
            break;
        case DrawCommand::RenderQueue::UI:
            targetQueue = &_uiCommandQueue;
            break;
        case DrawCommand::RenderQueue::INVALID:
        default:
            ENGINE_ASSERT(false, "Draw command requests a non-existing or invalid command queue (\"{}\").", magic_enum::enum_name(command.queue));
    }

    targetQueue->push_back(command);
}

void Renderer::renderFrame()
{
    ZoneScopedN("Renderer::renderFrame");

    glClearColor(_viewportData.clearColor.r, _viewportData.clearColor.g, _viewportData.clearColor.b, _viewportData.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _frameStats = FrameStats(); // Reset stats to 0

    const float currentTime = time::sinceLoad();

    // Upload instance data for all passes in the same buffer
    // TODO revisit this and see if an asynchronous buffer could work too?
    _instanceDataBuffer.clear();

    sortCommandList(_opaqueCommandQueue);
    sortCommandList(_uiCommandQueue);

    appendInstanceData(_opaqueCommandQueue);
    appendInstanceData(_uiCommandQueue);

    _instanceDataBuffer.upload();
    _instanceDataBuffer.bind(InstanceData::SHADER_BINDING);

    _frameStats.numCommands += _opaqueCommandQueue.size();
    _frameStats.numCommands += _uiCommandQueue.size();

    usize instanceIndex = 0;

    // Execute opaque pass
    PassDataBlock opaqueData;
    opaqueData.view = _viewportData.viewMatrix;
    opaqueData.projection = _viewportData.projectionMatrix;
    opaqueData.viewProj = _viewportData.getCombinedViewProjectionMatrix();
    opaqueData.time = currentTime;
    executePass(opaqueData, _opaqueCommandQueue, instanceIndex);
    instanceIndex += _opaqueCommandQueue.size();
    _opaqueCommandQueue.clear();

    // Execute UI pass
    PassDataBlock uiPassData;
    uiPassData.view = mat4::identity;
    uiPassData.projection = _viewportData.getScreenSpaceProjectionMatrix();
    uiPassData.viewProj = uiPassData.projection; // view matrix is identity, so view-projection is simply the projection mat
    uiPassData.time = currentTime;
    executePass(uiPassData, _uiCommandQueue, instanceIndex);
    instanceIndex += _uiCommandQueue.size();
    _uiCommandQueue.clear();
}

u64 Renderer::buildSortKey(assets::AssetRef<Material> material, assets::AssetRef<Mesh> mesh)
{
    return (static_cast<u64>(material->pipeline.sortKey) << 32) |
           (static_cast<u64>(material->sortKey) << 16) |
           (static_cast<u64>(mesh->sortKey));
}

void Renderer::bindPipeline(const Pipeline& pipeline)
{
    ZoneScopedN("Renderer::bindPipeline");

    // Shader
    glUseProgram(pipeline._programId);

    // Depth testing
    if (pipeline._descriptor.depthTest)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(gl_platform::getGlDepthFunc(pipeline._descriptor.depthFunc));
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    // Face culling
    switch (pipeline._descriptor.backfaceCulling)
    {
        case BackfaceCulling::Back:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case BackfaceCulling::Front:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case BackfaceCulling::None:
            glDisable(GL_CULL_FACE);
            break;
    }

    // Blend
    if (pipeline._descriptor.blend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(
            gl_platform::getGlBlendFuncOption(pipeline._descriptor.blendSource),
            gl_platform::getGlBlendFuncOption(pipeline._descriptor.blendDestination)
        );
    }
    else
    {
        glDisable(GL_BLEND);
    }

    _frameStats.numPipelineBinds++;
}

void Renderer::bindMaterial(assets::AssetRef<Material> material)
{
    ZoneScopedN("Renderer::bindMaterial");

    // Bind material ubo
    material->flushUboChangesToGpu();
    if (material->_layout.hasMaterialBlock())
    {
        const auto& blockInfo = material->_layout.getMaterialBlockInfo();
        // TODO block binding location is supposed to be 2 by convention so technically we could omit obtaining it from reflected layout, but then it should be validated on load.
        glBindBufferBase(GL_UNIFORM_BUFFER, blockInfo.binding, material->_uboHandle);
    }

    // Bind textures
    if (material->_textures.size() > 0)
    {
        for (const auto& [propertyId, textureRef] : material->_textures)
        {
            assets::AssetRef<Texture> textureToBind = textureRef;

            if (textureRef.isNull())
            {
                textureToBind = Texture::fallbackWhite();
            }

            const ShaderPropertyInfo* prop = material->_layout.getPropertyInfo(propertyId);
            ENGINE_ASSERT(prop != nullptr, "Trying to bind texture property (id:{}) from material that does not exist in shader layout. Material should not map properties that do not exist in the layout of its shader.", propertyId);

            const SamplerInfo& samplerInfo = prop->getSamplerInfo();
            glBindTextureUnit(samplerInfo.textureUnit, textureToBind->getGlBindPoint());
            glBindSampler(samplerInfo.textureUnit, _defaultSampler);
        }
    }

    // Bind buffers
    if (material->_buffers.size() > 0)
    {
        for (const auto& [propertyId, buffer] : material->_buffers)
        {
            const ShaderPropertyInfo* prop = material->_layout.getPropertyInfo(propertyId);
            ENGINE_ASSERT(prop != nullptr, "Trying to bind texture property (id:{}) from material that does not exist in shader layout. Material should not map properties that do not exist in the layout of its shader.", propertyId);

            const BufferInfo& bufferInfo = prop->getBufferInfo();
            if (buffer == nullptr)
            {
                // Unbind buffer at binding if no buffer is assigned to the property.
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferInfo.binding, 0);
                continue;
            }
            // NOTE: The user is responsible for managing the lifetime of data inside the buffer. Do not upload data here.
            // This could eventually be implemented as a failsafe, but a flag needs to exist on the buffer object to
            // check whether the data is synchronized.
            buffer->bind(bufferInfo.binding);
        }
    }
}

void Renderer::bindPassData(const PassDataBlock& passData) const
{
    glNamedBufferSubData(_frameDataUboHandle, 0, sizeof(PassDataBlock), &passData);
    glBindBufferBase(GL_UNIFORM_BUFFER, PassDataBlock::SHADER_BINDING, _frameDataUboHandle);
}

void Renderer::sortCommandList(CommandQueue& queue)
{
    ZoneScopedN("Command list sorting")
    std::ranges::sort(queue, { }, &DrawCommand::sortKey);
}

void Renderer::appendInstanceData(CommandQueue& queue)
{
    ZoneScopedN("Instance data fetch & upload")
    auto instanceDataFromDrawCommandView = queue | std::ranges::views::transform([](const auto& input)
    {
        InstanceData data;
        data.transform = input.modelMatrix;
        data.customData = input.customInstanceData;
        return data;
    });
    _instanceDataBuffer.appendRange(instanceDataFromDrawCommandView);
}

void Renderer::executePass(const PassDataBlock& passData, CommandQueue& queue, usize instanceIndex)
{
    if (queue.empty()) return;

    ZoneScopedN("Renderer::renderSceneGeometry");
    TracyGpuZone("Renderer::renderSceneGeometry");

    bindPassData(passData);

    u16 currentPipelineId = 0xFFFF;
    u16 currentMaterialId = 0xFFFF;
    u16 currentMeshId = 0xFFFF;

    usize batchStart = 0;
    while (batchStart < queue.size())
    {
        const DrawCommand& baseCommand = queue[batchStart];

        usize batchEnd = batchStart + 1;
        while (batchEnd < queue.size() &&
               queue[batchEnd].sortKey == baseCommand.sortKey)
        {
            batchEnd++;
        }

        const usize batchCount = batchEnd - batchStart;

        if (baseCommand.getPipelineId() != currentPipelineId)
        {
            bindPipeline(baseCommand.material->pipeline);
            currentPipelineId = baseCommand.getPipelineId();
        }

        if (baseCommand.getMaterialId() != currentMaterialId)
        {
            bindMaterial(baseCommand.material);
            currentMaterialId = baseCommand.getMaterialId();
        }

        if (baseCommand.getMeshId() != currentMeshId)
        {
            glBindVertexArray(baseCommand.mesh.vao);
            currentMeshId = baseCommand.getMeshId();
        }

        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, baseCommand.mesh.indexCount, GL_UNSIGNED_INT, nullptr, batchCount, batchStart + instanceIndex);
        _frameStats.numDrawCalls++;

        batchStart = batchEnd;
    }
}
