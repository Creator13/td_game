#include "Renderer.h"

#include <glad/glad.h>

#include "core/Constants.h"
#include "rendering/Mesh.h"
#include "rendering/shader.h"

using namespace math;
using namespace graphics;

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
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 vpMatrix = projectionMatrix * core::COORDINATE_BASIS * viewMatrix;

    for (size_t i = 0; i < renderables.size(); i++)
    {
        const Renderable& rObj = renderables[i];
        shader::use(assets::AssetDatabase::getShaderProgram(rObj.shaderId));

        const MeshGpuHandle& handle = assets::AssetDatabase::getMeshGpuHandle(rObj.meshId);

        glBindVertexArray(handle.vao);
        glUniformMatrix4fv(100, 1, GL_FALSE, rObj.modelMatrix.m);
        glUniformMatrix4fv(101, 1, GL_FALSE, vpMatrix.m);

        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    renderables.clear();
}

