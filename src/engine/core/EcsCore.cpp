#include "core/EcsCore.h"

#include <flecs.h>

#include "Input.h"
#include "core/Color.h"
#include "core/Transform.h"
#include "math/geom.h"

using namespace math;
using namespace core::ecs;

namespace
{
    void registerMathComponents(flecs::world& ecs)
    {
        // TODO possibly extend this into math module? idk not sure if that has added value...

        ecs.component<vec2>()
            .member<float>("x")
            .member<float>("y");

        ecs.component<vec3>()
            .member<float>("x")
            .member<float>("y")
            .member<float>("z");

        ecs.component<quaternion>()
            .member<float>("x")
            .member<float>("y")
            .member<float>("z")
            .member<float>("w");

        ecs.component<rect>()
            .member<vec2>("Offset")
            .member<vec2>("Size");
    }

    void registerComponents(flecs::world& ecs)
    {
        registerMathComponents(ecs);

        ecs.component<core::Color>()
            .member<float>("r")
            .member<float>("g")
            .member<float>("b")
            .member<float>("a");

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
        // TODO flecs::OnSet makes this *also* trigger when e.modified() is called, which would reset the values?
        //  Mayhaps this needs this to be a helper function instead?
        .event(flecs::OnSet)
        .each([](const HierarchyTransform& transform, FreeLookCameraControlData& cam)
        {
            const vec3 euler = toEuler(transform.getGlobalOrientation());
            cam.yaw = euler.z;
            cam.pitch = euler.x;
        });
}
