#include "EcsDebug.h"

#include "Debug.h"
#include "rendering/EcsRendering.h"

using namespace debug;

ecs::engine_debug::engine_debug(flecs::world& ecs)
{
    ecs.module<engine_debug>("Debug module");
    ecs.component<DebugRendererSingleton>().add(flecs::Singleton);

    ecs.system<const core::ecs::CameraRenderData, DebugRendererSingleton>()
        .each([](const core::ecs::CameraRenderData& renderData, DebugRendererSingleton& dbg)
        {
            dbg.ptr->setMatrices(renderData.viewMatrix, renderData.projectionMatrix);
        });
}
