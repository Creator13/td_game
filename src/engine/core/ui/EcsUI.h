#pragma once

#include <string>

#include "datatype.h"
#include "assets/AssetRef.h"
#include "math/geom.h"

namespace flecs
{
    struct world;
}

namespace core::ui
{
    struct UiRect
    {
        math::vec2 offset;
        math::vec2 size;
        float rotation;
    };

    struct UiRoot
    {
        u16 referenceWidth, referenceHeight;
    };

    struct TextData
    {
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
    };
}
