#pragma once

#include <array>

#include "math/vec2.h"

class GLFWwindow;

namespace core
{
    enum class Key : uint8_t
    {
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        Alpha1, Alpha2, Alpha3, Alpha4, Alpha5,
        Alpha6, Alpha7, Alpha8, Alpha9, Alpha0,

        Minus, Equal, LeftBracket, RightBracket, Backslash,
        Semicolon, Apostrophe, Grave, Comma, Period, Slash,

        Escape, Enter, Tab, Backspace, Space,

        LeftShift, RightShift,
        LeftControl, RightControl,
        LeftAlt, RightAlt,
        LeftSuper, RightSuper,
        Menu,

        F1, F2, F3, F4,
        F5, F6, F7, F8,
        F9, F10, F11, F12,
        F13, F14, F15, F16,

        Insert, Delete, Home, End, PageUp, PageDown,

        Up, Down, Left, Right,

        Numpad1, Numpad2, Numpad3, Numpad4, Numpad5,
        Numpad6, Numpad7, Numpad8, Numpad9, Numpad0,

        NumpadPeriod, NumpadDivide, NumpadMultiply, NumpadSubtract,
        NumpadAdd, NumpadEnter, NumpadEqual,

        NumLock, CapsLock, ScrollLock,

        PrintScreen, Pause,

        Unknown,

        // COUNT
        _Count
    };

    enum class ButtonAction : uint8_t
    {
        Press, Release,
    };

    enum class MouseButton : uint8_t
    {
        // Same mapping as glfw MOUSE_BUTTON_{1-8} which is just 0-7
        Left, Right, Middle, Mouse4, Mouse5, Mouse6, Mouse7, Mouse8, _Count
    };

    // GLFW callbacks
    void glfw_cursorPosCallback(GLFWwindow* window, double x, double y);
    void glfw_keyCallback(GLFWwindow* window, int token, int scancode, int action, int mods);
    void glfw_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    // Input manager
    struct InputState
    {
        math::vec2 currentMousePos;
        math::vec2 lastMousePos;
        math::vec2 mouseDelta;

        std::array<bool, static_cast<size_t>(MouseButton::_Count)> mouseStatePreviousFrame = {false};
        std::array<bool, static_cast<size_t>(MouseButton::_Count)> mouseStateCurrentFrame = {false};

        std::array<bool, static_cast<size_t>(Key::_Count)> keyStatePreviousFrame = {false};
        std::array<bool, static_cast<size_t>(Key::_Count)> keyStateCurrentFrame = {false};

        void endFrame();

        void handleCursorPosUpdate(math::vec2 newPos);
        void handleKeyAction(Key key, ButtonAction action);
        void handleMouseAction(MouseButton button, ButtonAction action);

        bool isKeyPressed(Key key, int modifiers) const;
        bool isKeyPressed(Key key) const;
        bool isKeyReleased(Key key) const;
        bool isKeyDown(Key key, int modifiers) const;
        bool isKeyDown(Key key) const;

        bool isMousePressed(MouseButton button) const;
        bool isMouseReleased(MouseButton button) const;
        bool isMouseDown(MouseButton button) const;
    };
}

namespace core::ecs
{
    struct GlobalInput
    {
        InputState* state;
    };
}
