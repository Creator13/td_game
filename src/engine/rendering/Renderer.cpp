#include "Renderer.h"

#include "shader.h"

void graphics::Renderer::setClearColor(color c)
{
    clearColor = c;
}

void graphics::Renderer::setVpMatrix(const math::mat4& m)
{
    vpMatrix = m;
}

void graphics::Renderer::submit(const Renderable& renderable)
{
    renderables.push_back(renderable);
}

void graphics::Renderer::render()
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < renderables.size(); i++)
    {
        const Renderable* rObj = &renderables[i];
        shader::use(rObj->shader);

        glBindVertexArray(rObj->model.vao);
        glUniformMatrix4fv(100, 1, GL_FALSE, rObj->modelMatrix.m);
        glUniformMatrix4fv(101, 1, GL_FALSE, vpMatrix.m);
        glUniform4fv(50, 1, &rObj->material.color.r);

        glDrawElements(GL_TRIANGLES, rObj->model.idxCount, GL_UNSIGNED_INT, nullptr);
    }

    renderables.clear();
}

