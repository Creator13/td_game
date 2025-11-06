#pragma once

#include <cstdint>
#include <vector>
#include <glad/glad.h>

#include "color.h"
#include "shader.h"
#include "math/mat4.h"

namespace graphics {
    struct color;
}

namespace graphics
{
    struct GpuMesh
    {
        GLuint vao, vbo, ebo;
        uint32_t idxCount;
    };

    struct MaterialData
    {
        color color;
    };

    struct Renderable
    {
        math::mat4 modelMatrix;
        GpuMesh model;
        shader::ShaderProgramData shader;
        MaterialData material;
    };

    class Renderer
    {
        std::vector<Renderable> renderables;
        color clearColor = color(0,0,0,0);
        math::mat4 vpMatrix = math::mat4::zero;

    public:
        void setClearColor(color c);
        void setVpMatrix(const math::mat4& m);
        void submit(const Renderable& renderable);
        void render();
    };
}
