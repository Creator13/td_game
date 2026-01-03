#include "core/EcsCore.h"

#include <flecs.h>
#include <spdlog/spdlog.h>

#include "Input.h"
#include "core/Transform.h"

using namespace math;
using namespace core::ecs;

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

        ecs.component<GlobalInput>().add(flecs::Singleton);
    }
}

engine_core::engine_core(flecs::world& ecs)
{
    ecs.module<engine_core>("Core module");
    registerComponents(ecs);

    ecs.observer<const HierarchyTransform, FreeLookCameraControlData>()
        .event(flecs::OnSet)
        .each([](flecs::entity e, const HierarchyTransform& transform, FreeLookCameraControlData& cam)
        {
            const vec3 euler = toEuler(transform.getGlobalOrientation());
            cam.yaw = euler.z;
            cam.pitch = euler.x;
        });
}
