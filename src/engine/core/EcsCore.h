#pragma once

namespace flecs {
    struct world;
}

namespace core::ecs
{
    struct FreeLookCameraControlData
    {
        float targetSpeed;
        float speed;
        float currentMoveDuration;
        float sensitivity = .1;

        float yaw, pitch;
    };

    struct engine_core
    {
        engine_core(flecs::world& ecs);
    };
}
