#pragma once

#include <variant>
#include <vector>

#include "assets/AssetDatabase.h"
#include "math/geom.h"
#include "math/mat4.h"
#include "rendering/Color.h"

class GLFWwindow;

namespace graphics
{
    struct Color;
}

namespace graphics
{
    struct MaterialData
    {
        Color color;
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

        bool drawDebug = false;

        std::vector<Renderable> renderables;
        Color clearColor = Color(0, 0, 0, 0);
        math::mat4 projectionMatrix;
        math::mat4 viewMatrix;

    public:
        void setAssetDatabase(const assets::AssetDatabase* db);

        void setClearColor(Color c);
        void setViewToClipMatrix(const math::mat4& m);
        void setWorldToViewMatrix(const math::mat4& m);

        void submit(const Renderable& renderable);

        void render();
    };
}
