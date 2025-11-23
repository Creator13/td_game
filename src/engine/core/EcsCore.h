#pragma once

namespace flecs {
    struct world;
}

namespace core::ecs
{
    struct engine_core
    {
        engine_core(flecs::world& ecs);
    };
}
