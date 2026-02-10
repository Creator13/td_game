#include "Renderer.h"

#include <glad/gl.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "assets/Shader.h"
#include "core/Constants.h"
#include "core/Mesh.h"
#include "rendering/Material.h"

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

    glCreateSamplers(1, &_sampler);
    glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Renderer::copyViewportData(const ViewportData& params)
{
    _viewportData = params;
}

void Renderer::submitSceneGeometry(const DrawCommand& renderable)
{
    _geometryCommandBuffer.push_back(renderable);
}

void Renderer::renderFrame()
{
    renderSceneGeometry();
}

void Renderer::setPass(const RenderPass& pass)
{
    (glEnable)(GL_DEPTH_TEST);

    (pass.depth ? glEnable : glDisable)(GL_DEPTH_TEST);

    switch (pass.backfaceCulling)
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

    // TODO maybe store pass reference in a field for reference during the pass? Dunno if that is needed or if I can
    //  just change specific state. Anyway this function will NOT be static-qualified in the future...
}

void Renderer::renderSceneGeometry()
{
    ZoneScopedN("Renderer::render()");
    TracyGpuZone("Renderer::render()");

    setPass(RenderPass{
        .depth = true,
        .backfaceCulling = BackfaceCulling::Back
    });

    glClearColor(_viewportData.clearColor.r, _viewportData.clearColor.g, _viewportData.clearColor.b, _viewportData.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const mat4 vpMatrix = _viewportData.getCombinedViewProjectionMatrix();

    for (usize i = 0; i < _geometryCommandBuffer.size(); i++)
    {
        const DrawCommand& rObj = _geometryCommandBuffer[i];

        glUseProgram(rObj.material->shader->programId);

        const gpu::MeshGpuHandle& handle = rObj.mesh->gpuHandle;
        glBindVertexArray(handle.vao);

        glBindTextureUnit(0, rObj.material->albedo->getGlBindPoint());
        glBindSampler(0, _sampler);

        glUniformMatrix4fv(100, 1, GL_FALSE, rObj.modelMatrix.m);
        glUniformMatrix4fv(101, 1, GL_FALSE, vpMatrix.m);

        glUniform1i(110, 0);

        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    _geometryCommandBuffer.clear();
}
