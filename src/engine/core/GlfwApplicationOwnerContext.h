#pragma once

namespace core
{
    struct WindowState;
    struct InputState;

    struct GlfwApplicationOwnerContext
    {
        InputState* input;
        WindowState* window;
    };
}
