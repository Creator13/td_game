#include "Renderer.h"

#include <glad/glad.h>

#include "rendering/Mesh.h"
#include "rendering/shader.h"

using namespace math;
using namespace graphics;

// Define a z+ up, y+ forward coordinate system
constexpr mat4 coordinateBasis = mat4(
    1, 0, 0, 0,
    0, 0, 1, 0,
    0, -1, 0, 0,
    0, 0, 0, 1
);

void Renderer::setAssetDatabase(const assets::AssetDatabase* db)
{
    this->db = db;
}

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

    mat4 vpMatrix = projectionMatrix * coordinateBasis * viewMatrix;

    for (size_t i = 0; i < renderables.size(); i++)
    {
        const Renderable& rObj = renderables[i];
        shader::use(db->getShaderProgram(rObj.shaderId));

        const MeshGpuHandle& handle = db->getMeshGpuHandle(rObj.meshId);

        glBindVertexArray(handle.vao);
        glUniformMatrix4fv(100, 1, GL_FALSE, rObj.modelMatrix.m);
        glUniformMatrix4fv(101, 1, GL_FALSE, vpMatrix.m);
        glUniform4fv(50, 1, &rObj.material.color.r);

        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    renderables.clear();
}

