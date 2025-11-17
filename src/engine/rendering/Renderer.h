#pragma once

#include <vector>

#include "color.h"
#include "shader.h"
#include "assets/AssetDatabase.h"
#include "math/mat4.h"

class GLFWwindow;

namespace graphics
{
    struct color;
}

namespace graphics
{
    struct MaterialData
    {
        color color;
    };

    struct Renderable
    {
        math::mat4 modelMatrix;
        assets::AssetId meshId;
        assets::AssetId shaderId;
        MaterialData material;
    };

    static_assert(std::is_trivially_destructible_v<Renderable>);

    class Renderer
    {
        const assets::AssetDatabase* db = nullptr;

        std::vector<Renderable> renderables;
        color clearColor = color(0, 0, 0, 0);
        math::mat4 projectionMatrix;
        math::mat4 viewMatrix;

    public:
        void setAssetDatabase(const assets::AssetDatabase* db);
        void setClearColor(color c);
        void setViewToClipMatrix(const math::mat4& m);
        void setWorldToViewMatrix(const math::mat4& m);
        void submit(const Renderable& renderable);
        void render();
    };
}
