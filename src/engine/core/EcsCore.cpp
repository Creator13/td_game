#include "core/EcsCore.h"

#include <flecs.h>

#include "Input.h"
#include "core/Transform.h"

using namespace math;
using namespace core;

namespace
{
    void registerComponents(flecs::world& ecs)
    {
        ecs.component<vec3>()
            .member<float>("x")
            .member<float>("y")
            .member<float>("z");

        ecs.component<quaternion>()
            .member<float>("x")
            .member<float>("y")
            .member<float>("z")
            .member<float>("w");

        ecs.component<TransformHandle>()
            .on_remove([](flecs::entity e, TransformHandle& t)
            {
                // TODO return id to pool
            })
            .member<TransformIndex>("Transform ID");

        ecs.component<ecs::GlobalInput>().add(flecs::Singleton);
    }
}

ecs::engine_core::engine_core(flecs::world& ecs)
{
    ecs.module<engine_core>("Core module");
    registerComponents(ecs);
}
