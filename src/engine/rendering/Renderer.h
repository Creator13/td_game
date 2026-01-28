#pragma once

#include <variant>
#include <vector>

#include "assets/AssetDatabase.h"
#include "assets/Shader.h"
#include "math/mat4.h"
#include "rendering/Color.h"
#include "rendering/Mesh.h"

namespace core {
    struct Material;
}

class GLFWwindow;

namespace graphics
{
    struct Renderable
    {
        math::mat4 modelMatrix;
        core::assets::AssetRef<core::Mesh> mesh;
        core::Material* material;
    };

    static_assert(std::is_trivially_destructible_v<Renderable>);

    class Renderer
    {
        bool drawDebug = false;

        std::vector<Renderable> renderables;
        ColorLinear clearColor = ColorLinear();
        math::mat4 projectionMatrix;
        math::mat4 viewMatrix;

        core::gl::Uint sampler;

    public:
        Renderer();

        void setClearColor(ColorLinear c);
        void setViewToClipMatrix(const math::mat4& m);
        void setWorldToViewMatrix(const math::mat4& m);

        void submit(const Renderable& renderable);

        void render();
    };
}
