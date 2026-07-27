#include "Renderer.h"

#include <glad/gl.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "assets/Mesh.h"
#include "assets/ShaderLoader.h"
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

// BELOW FUNCTIONS ARE LLM GENERATED; THEY DON'T WORK AMAZINGLY.

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
    : _instanceDataBuffer(1_MB),
      _mainFramebuffer(800, 600, TextureFormat::RGBA16_FLOAT, true),
      _pingPongFramebuffers({
          Framebuffer(800, 600, TextureFormat::RGBA8_UNORM, false),
          Framebuffer(800, 600, TextureFormat::RGBA8_UNORM, false)
      })
{
    gl::Int maxUboBindings, maxSsboBindings, maxTextureBindings, maxImageBindings;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUboBindings);
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxSsboBindings);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureBindings);
    glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageBindings);
    SPDLOG_DEBUG("Max shader object bindings: UBO|{} - SSBO|{} - Tex|{} - Img|{}", maxUboBindings, maxSsboBindings, maxTextureBindings, maxImageBindings);
}

void Renderer::init(int fbWidth, int fbHeight)
{
    glFrontFace(GL_CCW);

    resizeFrameBuffers(fbWidth, fbHeight);

    auto fsPipeline = Pipeline::createFullscreenEffect("Fullscreen blit", "shaders/fullscreen/blit.fs.glsl");
    _fullscreenBlitEffect = fsPipeline->newMaterialInstance("anonymous");

    glCreateBuffers(1, &_viewportDataUboHandle.id);
    glNamedBufferStorage(_viewportDataUboHandle, sizeof(ViewportDataBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &_frameDataUboHandle.id);
    glNamedBufferStorage(_frameDataUboHandle, sizeof(FrameDataBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &_lightingDataUboHandle.id);
    glNamedBufferStorage(_lightingDataUboHandle, sizeof(LightingDataBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);

    glCreateSamplers(1, &_defaultSampler);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(_defaultSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glCreateVertexArrays(1, &_emptyVao.id);
}

void Renderer::setViewportData(const ViewportData& params)
{
    if (_viewportData.pixelHeight != params.pixelHeight || _viewportData.pixelWidth != params.pixelWidth)
    {
        resizeFrameBuffers(params.pixelWidth, params.pixelHeight);
    }

    _viewportData = params;
}

void Renderer::submitPointLight(const PointLight& light)
{
    _pointLights.push_back(light);
}

void Renderer::submitDirectionalLight(const DirectionalLight& light)
{
    _dirLights.push_back(light);
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

void Renderer::setEnvironmentSettings(const EnvironmentSettings& env) { }

void Renderer::renderFrame()
{
    ZoneScopedN("Renderer::renderFrame");

    static_assert(std::is_trivially_destructible_v<FrameStats>);
    _frameStats = FrameStats{ };

    FrameDataBlock frameData;
    frameData.screenSize = vec2(_viewportData.pixelWidth, _viewportData.pixelHeight);
    frameData.time = time::sinceLoad();
    bindFrameData(frameData);

    const DirectionalLight& firstDirLight = _dirLights[0];
    LightingDataBlock lightingData;
    lightingData.ambientStrength = _environmentSettings.ambientIntensity;
    lightingData.numPointLights = min(_pointLights.size(), 8);
    for (int i = 0; i < lightingData.numPointLights; i++)
    {
        lightingData.pointLights[i] = {
            .position = _pointLights[i].position,
            .color = _pointLights[i].color.rgbVec3(),
            .intensity = _pointLights[i].intensity,
        };
    }
    if (_dirLights.size() > 0)
    {
        lightingData.dirLight = {
            .direction = firstDirLight.direction,
            .color = firstDirLight.color.rgbVec3(),
            .intensity = firstDirLight.intensity,
        };
    }
    bindLightingData(lightingData);

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

    glBindFramebuffer(GL_FRAMEBUFFER, _mainFramebuffer._fbo);
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Execute opaque pass
    ViewportDataBlock opaqueData;
    opaqueData.view = _viewportData.viewMatrix;
    opaqueData.projection = _viewportData.projectionMatrix;
    opaqueData.viewProj = _viewportData.getCombinedViewProjectionMatrix();
    opaqueData.cameraPos = _viewportData.cameraPos;
    executePass(opaqueData, _opaqueCommandQueue, instanceIndex);
    instanceIndex += _opaqueCommandQueue.size();
    _opaqueCommandQueue.clear();

    executePostEffectStack();

    // Execute UI pass
    ViewportDataBlock uiPassData;
    uiPassData.view = mat4::identity;
    uiPassData.projection = _viewportData.getScreenSpaceProjectionMatrix();
    uiPassData.viewProj = uiPassData.projection; // view matrix is identity, so view-projection is simply the projection mat
    uiPassData.cameraPos = vec3::zero; // Not rendering with a specific camera position, so we just set this to the origin.
    glEnable(GL_FRAMEBUFFER_SRGB);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Render UI as overlay to the current image in the backbuffer (the scene)
    executePass(uiPassData, _uiCommandQueue, instanceIndex);
    glDisable(GL_FRAMEBUFFER_SRGB);
    instanceIndex += _uiCommandQueue.size();
    _uiCommandQueue.clear();

    // Frame cleanup
    _pointLights.clear();
}

void Renderer::setPostEffectStack(const std::vector<assets::AssetRef<Material>>& stack)
{
    _postEffects.clear();
    _postEffects.assign_range(stack);
}

void Renderer::setClearColor(const Color& clearColor)
{
    _clearColor = clearColor;
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
        glBindBufferBase(GL_UNIFORM_BUFFER, MaterialBlock::SHADER_BINDING, material->_uboHandle);
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

void Renderer::bindPassData(const ViewportDataBlock& passData) const
{
    glNamedBufferSubData(_viewportDataUboHandle, 0, sizeof(ViewportDataBlock), &passData);
    glBindBufferBase(GL_UNIFORM_BUFFER, ViewportDataBlock::SHADER_BINDING, _viewportDataUboHandle);
}

void Renderer::bindFrameData(const FrameDataBlock& passData) const
{
    glNamedBufferSubData(_frameDataUboHandle, 0, sizeof(FrameDataBlock), &passData);
    glBindBufferBase(GL_UNIFORM_BUFFER, FrameDataBlock::SHADER_BINDING, _frameDataUboHandle);
}

void Renderer::bindLightingData(const LightingDataBlock& lightingData) const
{
    glNamedBufferSubData(_lightingDataUboHandle, 0, sizeof(LightingDataBlock), &lightingData);
    glBindBufferBase(GL_UNIFORM_BUFFER, LightingDataBlock::SHADER_BINDING, _lightingDataUboHandle);
}

void Renderer::resizeFrameBuffers(int newWidth, int newHeight)
{
    ZoneScopedN("Framebuffer resize");
    _mainFramebuffer.setSize(newWidth, newHeight);
    _mainFramebuffer.create();

    _pingPongFramebuffers[0].setSize(newWidth, newHeight);
    _pingPongFramebuffers[0].create();
    _pingPongFramebuffers[1].setSize(newWidth, newHeight);
    _pingPongFramebuffers[1].create();
}

void Renderer::appendInstanceData(CommandQueue& queue)
{
    ZoneScopedN("Instance data fetch & upload")
    auto instanceDataFromDrawCommandView = queue | std::ranges::views::transform([](const auto& input)
    {
        return input.instanceData;
    });
    _instanceDataBuffer.appendRange(instanceDataFromDrawCommandView);
}

void Renderer::executePass(const ViewportDataBlock& passData, CommandQueue& queue, usize instanceIndex)
{
    if (queue.empty()) return;

    ZoneScopedN("Pass execution");
    TracyGpuZone("Pass execution");

    bindPassData(passData);

    // TODO why do these still start at ffff, and not just zero? Isn't that a leftover from some older design?
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

        _frameStats.triCount += (baseCommand.mesh.indexCount / 3) * batchCount;

        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, baseCommand.mesh.indexCount, GL_UNSIGNED_INT, nullptr, batchCount, batchStart + instanceIndex);
        _frameStats.numDrawCalls++;

        batchStart = batchEnd;
    }
}

void Renderer::executePostEffectStack()
{
    ZoneScopedN("Execute post processing stack")

    if (_postEffects.empty())
    {
        executePostEffect(_fullscreenBlitEffect, _mainFramebuffer, 0);
        return;
    }

    Framebuffer* src = &_mainFramebuffer;
    for (usize i = 0; i < _postEffects.size() - 1; ++i)
    {
        Framebuffer& target = _pingPongFramebuffers[i % 2];
        executePostEffect(_postEffects[i], *src, target._fbo);
        src = &target;
    }

    // Last effect uses the last destination as source and writes to the backbuffer
    executePostEffect(_postEffects[_postEffects.size() - 1], *src, 0);
}

void Renderer::executePostEffect(assets::AssetRef<Material> material, const Framebuffer& src, gl::framebuffer_t dst)
{
    ZoneScopedN("Apply effect")
    TracyGpuZone("Apply effect");

    bindPipeline(material->pipeline);
    material->setTexture2D("_screenTexture"_spid, src._colorAttachment);
    bindMaterial(material);
    glBindFramebuffer(GL_FRAMEBUFFER, dst);
    glBindVertexArray(_emptyVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    _frameStats.numDrawCalls++;
}

void Renderer::sortCommandList(CommandQueue& queue)
{
    ZoneScopedN("Command list sorting")
    std::ranges::sort(queue, { }, &DrawCommand::sortKey);
}
