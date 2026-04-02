#pragma once
#include <flecs.h>

namespace core {
    struct TransformSystem;
    struct WindowState;
}

namespace engine
{
    extern core::WindowState getInitialWindowState();
    extern void setupGame(const flecs::world& world);
}