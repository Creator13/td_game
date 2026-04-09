#pragma once

#include <string>

#include "datatype.h"
#include "assets/AssetRef.h"
#include "assets/Mesh.h"
#include "rendering/Material.h"

namespace flecs
{
    struct world;
}

namespace core::ui
{
    struct Rect
    {
        math::vec2 offset;
        math::vec2 size;
        float rotation;
    };

    struct UiRoot
    {
        u16 referenceWidth, referenceHeight;
    };

    struct Text
    {
        explicit Text(std::string_view text);
        std::string text;
    };

    struct TextRenderData
    {
        // AssetRef<Font> font
        float size;
    };

    struct engine_ui
    {
        explicit engine_ui(flecs::world& ecs);

    private:
        assets::AssetRef<Mesh> _uiQuad;
        assets::AssetRef<Material> _uiMaterial;
    };
}
