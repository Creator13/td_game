#pragma once

#include <string>

#include "datatype.h"
#include "assets/AssetRef.h"
#include "assets/Font.h"
#include "assets/Mesh.h"
#include "rendering/Material.h"

namespace flecs
{
    struct world;
}

namespace core::ui
{
    namespace anchor
    {
        constexpr math::vec2 topLeft = math::vec2(0.0f, 0.0f);
        constexpr math::vec2 topCenter = math::vec2(0.5f, 0.0f);
        constexpr math::vec2 topRight = math::vec2(1.0f, 0.0f);

        constexpr math::vec2 middleLeft = math::vec2(0.0f, 0.5f);
        constexpr math::vec2 middleCenter = math::vec2(0.5f, 0.5f);
        constexpr math::vec2 middleRight = math::vec2(1.0f, 0.5f);

        constexpr math::vec2 bottomLeft = math::vec2(0.0f, 1.0f);
        constexpr math::vec2 bottomCenter = math::vec2(0.5f, 1.0f);
        constexpr math::vec2 bottomRight = math::vec2(1.0f, 1.0f);
    }

    struct Rect
    {
        math::vec2 offset;
        math::vec2 size;
        float rotation;
        math::vec2 anchor = anchor::middleCenter;

        operator math::rect() const { return math::rect(offset, size); }
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
        assets::AssetRef<Font> font;
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
