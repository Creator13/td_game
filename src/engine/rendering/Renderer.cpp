#include "Renderer.h"

#include <glad/gl.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "core/Constants.h"
#include "rendering/Mesh.h"
#include "../assets/Shader.h"
#include "rendering/Material.h"

using namespace math;
using namespace graphics;
using namespace core;

Renderer::Renderer()
{
    glCreateSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Renderer::setClearColor(ColorLinear c)
{
    clearColor = c;
}

void Renderer::setViewToClipMatrix(const mat4& m)
{
    projectionMatrix = m;
}

void Renderer::setWorldToViewMatrix(const mat4& m)
{
    viewMatrix = m;
}

void Renderer::submit(const Renderable& renderable)
{
    renderables.push_back(renderable);
}

void Renderer::render()
{
    ZoneScopedN("Renderer::render()");
    TracyGpuZone("Renderer::render()");

    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 vpMatrix = projectionMatrix * constants::COORDINATE_BASIS * viewMatrix;

    for (usize i = 0; i < renderables.size(); i++)
    {
        const Renderable& rObj = renderables[i];

        glUseProgram(rObj.material->shader->programId);

        const gpu::MeshGpuHandle& handle = rObj.mesh->gpuHandle;
        glBindVertexArray(handle.vao);

        glBindTextureUnit(0, rObj.material->albedo->getGlBindPoint());
        glBindSampler(0, sampler);

        glUniformMatrix4fv(100, 1, GL_FALSE, rObj.modelMatrix.m);
        glUniformMatrix4fv(101, 1, GL_FALSE, vpMatrix.m);

        glUniform1i(110, 0);

        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    renderables.clear();
}
