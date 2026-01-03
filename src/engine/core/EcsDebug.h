#pragma once
#include <flecs.h>

namespace core::debug
{
    struct DebugRenderer;
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
    };
}
