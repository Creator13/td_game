#include "Renderer.h"

#include <__msvc_ranges_to.hpp>
#include <glad/gl.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "core/Constants.h"
#include "core/Mesh.h"
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

Renderer::Renderer()
    : _perFrameUbo(4_MB)
{
    glFrontFace(GL_CCW);
    glEnable(GL_FRAMEBUFFER_SRGB); // Set *default* framebuffer to convert back to srgb on present

    glCreateBuffers(1, &_frameDataUboHandle.id);
    glNamedBufferStorage(_frameDataUboHandle, sizeof(FrameDataBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);

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

void Renderer::submitSceneGeometry(const DrawCommand& command)
{
    _geometryCommandBuffer.push_back(command);
}

void Renderer::renderFrame()
{
    ZoneScopedN("Renderer::renderFrame");

    glClearColor(_viewportData.clearColor.r, _viewportData.clearColor.g, _viewportData.clearColor.b, _viewportData.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSceneGeometry();
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
        glDepthFunc(pipeline._descriptor.depthFunc);
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
        glBlendFunc(pipeline._descriptor.blendSource, pipeline._descriptor.blendDestination);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void Renderer::bindMaterial(assets::AssetRef<Material> material)
{
    ZoneScopedN("Renderer::bindMaterial");

    // Bind material ubo
    material->flushUboChangesToGpu();
    if (material->_layout.hasMaterialBlock())
    {
        const auto& blockInfo =  material->_layout.getMaterialBlockInfo();
        // TODO block binding location is supposed to be 2 by convention so technically we could omit obtaining it from reflected layout, but then it should be validated on load.
        glBindBufferBase(GL_UNIFORM_BUFFER, blockInfo.binding, material->_uboHandle);
    }

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

void Renderer::bindFrameData() const
{
    FrameDataBlock frameDataBlock;
    frameDataBlock.view = _viewportData.viewMatrix;
    frameDataBlock.projection = _viewportData.projectionMatrix;
    frameDataBlock.viewProj = _viewportData.getCombinedViewProjectionMatrix();
    frameDataBlock.time = time::sinceLoad();

    glNamedBufferSubData(_frameDataUboHandle, 0, sizeof(FrameDataBlock), &frameDataBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, _frameDataUboHandle);
}

void Renderer::renderSceneGeometry()
{
    ZoneScopedN("Renderer::renderSceneGeometry");
    TracyGpuZone("Renderer::renderSceneGeometry");

    bindFrameData();

    {
        ZoneScopedN("sorting")
        std::ranges::sort(_geometryCommandBuffer, {}, &DrawCommand::sortKey);
    }

    {
        ZoneScopedN("command data upload")
        auto drawCommandToPerDrawDataView = _geometryCommandBuffer | std::ranges::views::transform([](const auto& input)
        {
            return PerDrawBlock{input.modelMatrix};
        });
        _perFrameUbo.alignAndUpload(drawCommandToPerDrawDataView);
    }

    u64 currentSortKey = 0;
    for (usize i = 0; i < _geometryCommandBuffer.size(); i++)
    {
        const DrawCommand& cmd = _geometryCommandBuffer[i];

        if (cmd.sortKey > currentSortKey)
        {
            bindPipeline(cmd.material->pipeline);
            bindMaterial(cmd.material);
            currentSortKey = cmd.sortKey;
        }

        _perFrameUbo.bindIndex(i, PerDrawBlock::BINDING);

        const gpu::MeshGpuHandle& handle = cmd.mesh->gpuHandle;
        glBindVertexArray(handle.vao);

        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    _geometryCommandBuffer.clear();
}

void Renderer::renderUI()
{
    ZoneScopedN("Renderer::renderSceneGeometry()");
    TracyGpuZone("Renderer::renderSceneGeometry()");
}
