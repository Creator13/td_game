#include "EcsUI.h"

#include <flecs.h>

#include "core/EcsCore.h"

namespace
{
    void registerComponents(flecs::world& ecs)
    {

    }
}

core::ui::engine_ui::engine_ui(flecs::world& ecs)
{
    ecs.import<ecs::engine_core>();

    registerComponents(ecs);
}
