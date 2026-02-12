#pragma once
#include "datatype.h"
#include "math/vec2.h"

namespace flecs
{
    struct world;
}

namespace core::ui
{
    struct UiTransform
    {
        math::vec2 position;
        math::vec2 scale;
        float rotation;
    };

    struct UiRoot
    {
        u16 referenceWidth, referenceHeight;
    };

    struct Panel { };

    struct engine_ui
    {
        engine_ui(flecs::world& ecs);
    };
}
