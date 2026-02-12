#include "EcsUI.h"

namespace
{
    void registerComponents(flecs::world& ecs)
    {

    }
}

core::ui::engine_ui::engine_ui(flecs::world& ecs)
{
    registerComponents(ecs);
}
