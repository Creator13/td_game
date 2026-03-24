#include "Renderer.h"

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
{
    glFrontFace(GL_CCW);
    glEnable(GL_FRAMEBUFFER_SRGB); // Set *default* framebuffer to convert back to srgb on present

    glCreateBuffers(1, &_frameDataUboHandle.id);
    glNamedBufferStorage(_frameDataUboHandle, sizeof(FrameDataBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);

    // glCreateSamplers(1, &_sampler);
    // glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Renderer::setViewportData(const ViewportData& params)
{
    _viewportData = params;
}

void Renderer::submitSceneGeometry(const DrawCommand& renderable)
{
    _geometryCommandBuffer.push_back(renderable);
}

void Renderer::renderFrame()
{
    glClearColor(_viewportData.clearColor.r, _viewportData.clearColor.g, _viewportData.clearColor.b, _viewportData.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSceneGeometry();
}

void Renderer::bindPipeline(const Pipeline& pipeline)
{
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

void Renderer::bindMaterial(Material& material)
{
    material.flushChangesToGpu();

    if (!material._layout.hasMaterialBlock()) return;

    const auto& blockInfo =  material._layout.getMaterialBlockInfo();
    // TODO binding is supposed to be 1 by convention so technically we could omit obtaining it from reflected layout, but then it should be validated on load.
    glBindBufferBase(GL_UNIFORM_BUFFER, blockInfo.binding, material._uboHandle);
}

void Renderer::bindFrameData() const
{
    FrameDataBlock frameDataBlock;
    frameDataBlock.viewProj = _viewportData.getCombinedViewProjectionMatrix();
    frameDataBlock.time = time::sinceLoad();

    glNamedBufferSubData(_frameDataUboHandle, 0, sizeof(FrameDataBlock), &frameDataBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, _frameDataUboHandle);
}

void Renderer::renderSceneGeometry()
{
    ZoneScopedN("Renderer::renderSceneGeometry()");
    TracyGpuZone("Renderer::renderSceneGeometry()");

    bindFrameData();

    for (usize i = 0; i < _geometryCommandBuffer.size(); i++)
    {
        const DrawCommand& cmd = _geometryCommandBuffer[i];

        bindPipeline(cmd.material->_pipeline);
        bindMaterial(*cmd.material);

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
