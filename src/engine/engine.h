#pragma once
#include <flecs.h>

namespace core {
    struct TransformSystem;
    struct WindowState;
}

namespace engine
{
    extern core::WindowState initial_window_state();
    extern void register_flecs(const flecs::world& world);
}