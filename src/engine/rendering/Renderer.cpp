#include "Renderer.h"

#include <glad/glad.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "core/Constants.h"
#include "rendering/Mesh.h"
#include "../assets/Shader.h"

using namespace math;
using namespace graphics;
using namespace core;

void Renderer::setClearColor(Color c)
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

    gl::program_t latestProgram;
    gl::vert_arr_t latestVao;

    for (usize i = 0; i < renderables.size(); i++)
    {
        const Renderable& rObj = renderables[i];

        if (rObj.shader->programId != latestProgram)
        {
            glUseProgram(rObj.shader->programId);
            latestProgram = rObj.shader->programId;
        }

        // Store handle straight in Renderable struct
        const gpu::MeshGpuHandle& handle = rObj.mesh->gpuHandle;

        if (handle.vao != latestVao)
        {
            glBindVertexArray(handle.vao);
            latestVao = handle.vao;
        }

        glUniformMatrix4fv(100, 1, GL_FALSE, rObj.modelMatrix.m);
        glUniformMatrix4fv(101, 1, GL_FALSE, vpMatrix.m);

        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    renderables.clear();
}