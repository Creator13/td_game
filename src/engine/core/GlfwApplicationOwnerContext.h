#pragma once

namespace core
{
    // fwd declarations, avoid circular dependencies
    struct WindowState;
    struct InputState;
    // ----

    struct GlfwApplicationOwnerContext
    {
        InputState* input;
        WindowState* window;
    };
}
