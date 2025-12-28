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

        ecs.component<HierarchyTransform>()
            .member<vec3>("Local position")
            .member<quaternion>("Local rotation")
            .member<vec3>("Local scale");

        ecs.component<ecs::GlobalInput>().add(flecs::Singleton);
    }
}

ecs::engine_core::engine_core(flecs::world& ecs)
{
    ecs.module<engine_core>("Core module");
    registerComponents(ecs);
}
