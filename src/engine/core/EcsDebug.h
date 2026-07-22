#pragma once
#include <flecs.h>

#include "assets/AssetRef.h"
#include "assets/Font.h"

namespace core::debug
{
    class DebugRenderer;
}

namespace debug::ecs
{
    struct DebugRendererSingleton
    {
        core::debug::DebugRenderer* ptr;
    };

    struct engine_debug
    {
        engine_debug(flecs::world& ecs);

    private:
        core::assets::AssetRef<core::Font> _debugInfoFont;
    };
}
