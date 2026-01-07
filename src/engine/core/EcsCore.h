#pragma once
#include "math/vec3.h"

namespace flecs {
    struct world;
}

namespace core::ecs
{
    struct FreeLookCameraControlData
    {
        float targetSpeed;
        float speedMultiplier;
        float sensitivity = .12f;

        math::vec3 currentVelocity;
        float yaw, pitch;
    };

    struct engine_core
    {
        engine_core(flecs::world& ecs);
    };
}
