#pragma once

#include "datatype.h"
#include "math/vec3.h"

namespace flecs
{
    struct world;
}

namespace core::ecs
{
    struct FreeLookCameraControlData
    {
        f32 targetSpeed;
        f32 speedMultiplier;
        f32 sensitivity = .12f;

        math::vec3 currentVelocity;
        f32 yaw, pitch;
    };

    struct engine_core
    {
        engine_core(flecs::world& ecs);
    };
}
