#include "EcsDebug.h"

#include "Debug.h"

using namespace debug;

ecs::engine_debug::engine_debug(flecs::world& ecs)
{
    ecs.module<engine_debug>("Debug module");
    ecs.component<DebugRendererSingleton>().add(flecs::Singleton);

    ecs.system<const core::gfx::ViewportData, DebugRendererSingleton>()
        .each([](const core::gfx::ViewportData& viewportData, DebugRendererSingleton& dbg)
        {
            dbg.ptr->copyViewportData(viewportData);
        });
}
