#include "core/Input.h"

#include <array>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "core/GlfwApplicationOwnerContext.h"

using namespace core;
using namespace math;

namespace
{
    constexpr size_t GLFW_KEYMAP_SIZE = 384;

    constexpr std::array<Key, GLFW_KEYMAP_SIZE> buildGlfwKeyTokenMap()
    {
        std::array<Key, GLFW_KEYMAP_SIZE> table;

        for (int i = 0; i < GLFW_KEYMAP_SIZE; i++)
        {
            table[i] = Key::Unknown;
        }

        table[GLFW_KEY_A] = Key::A;
        table[GLFW_KEY_B] = Key::B;
        table[GLFW_KEY_C] = Key::C;
        table[GLFW_KEY_D] = Key::D;
        table[GLFW_KEY_E] = Key::E;
        table[GLFW_KEY_F] = Key::F;
        table[GLFW_KEY_G] = Key::G;
        table[GLFW_KEY_H] = Key::H;
        table[GLFW_KEY_I] = Key::I;
        table[GLFW_KEY_J] = Key::J;
        table[GLFW_KEY_K] = Key::K;
        table[GLFW_KEY_L] = Key::L;
        table[GLFW_KEY_M] = Key::M;
        table[GLFW_KEY_N] = Key::N;
        table[GLFW_KEY_O] = Key::O;
        table[GLFW_KEY_P] = Key::P;
        table[GLFW_KEY_Q] = Key::Q;
        table[GLFW_KEY_R] = Key::R;
        table[GLFW_KEY_S] = Key::S;
        table[GLFW_KEY_T] = Key::T;
        table[GLFW_KEY_U] = Key::U;
        table[GLFW_KEY_V] = Key::V;
        table[GLFW_KEY_W] = Key::W;
        table[GLFW_KEY_X] = Key::X;
        table[GLFW_KEY_Y] = Key::Y;
        table[GLFW_KEY_Z] = Key::Z;

        table[GLFW_KEY_1] = Key::Alpha1;
        table[GLFW_KEY_2] = Key::Alpha2;
        table[GLFW_KEY_3] = Key::Alpha3;
        table[GLFW_KEY_4] = Key::Alpha4;
        table[GLFW_KEY_5] = Key::Alpha5;
        table[GLFW_KEY_6] = Key::Alpha6;
        table[GLFW_KEY_7] = Key::Alpha7;
        table[GLFW_KEY_8] = Key::Alpha8;
        table[GLFW_KEY_9] = Key::Alpha9;
        table[GLFW_KEY_0] = Key::Alpha0;

        table[GLFW_KEY_MINUS] = Key::Minus;
        table[GLFW_KEY_EQUAL] = Key::Equal;
        table[GLFW_KEY_LEFT_BRACKET] = Key::LeftBracket;
        table[GLFW_KEY_RIGHT_BRACKET] = Key::RightBracket;
        table[GLFW_KEY_BACKSLASH] = Key::Backslash;
        table[GLFW_KEY_SEMICOLON] = Key::Semicolon;
        table[GLFW_KEY_GRAVE_ACCENT] = Key::Grave;
        table[GLFW_KEY_COMMA] = Key::Comma;
        table[GLFW_KEY_PERIOD] = Key::Period;
        table[GLFW_KEY_SLASH] = Key::Slash;

        table[GLFW_KEY_ESCAPE] = Key::Escape;
        table[GLFW_KEY_ENTER] = Key::Enter;
        table[GLFW_KEY_TAB] = Key::Tab;
        table[GLFW_KEY_BACKSPACE] = Key::Backspace;
        table[GLFW_KEY_SPACE] = Key::Space;

        table[GLFW_KEY_LEFT_SHIFT] = Key::LeftShift;
        table[GLFW_KEY_RIGHT_SHIFT] = Key::RightShift;
        table[GLFW_KEY_LEFT_CONTROL] = Key::LeftControl;
        table[GLFW_KEY_RIGHT_CONTROL] = Key::RightControl;
        table[GLFW_KEY_LEFT_ALT] = Key::LeftAlt;
        table[GLFW_KEY_RIGHT_ALT] = Key::RightAlt;
        table[GLFW_KEY_LEFT_SUPER] = Key::LeftSuper;
        table[GLFW_KEY_RIGHT_SUPER] = Key::RightSuper;
        table[GLFW_KEY_MENU] = Key::Menu;

        table[GLFW_KEY_F1] = Key::F1;
        table[GLFW_KEY_F2] = Key::F2;
        table[GLFW_KEY_F3] = Key::F3;
        table[GLFW_KEY_F4] = Key::F4;
        table[GLFW_KEY_F5] = Key::F5;
        table[GLFW_KEY_F6] = Key::F6;
        table[GLFW_KEY_F7] = Key::F7;
        table[GLFW_KEY_F8] = Key::F8;
        table[GLFW_KEY_F9] = Key::F9;
        table[GLFW_KEY_F10] = Key::F10;
        table[GLFW_KEY_F11] = Key::F11;
        table[GLFW_KEY_F12] = Key::F12;
        table[GLFW_KEY_F13] = Key::F13;
        table[GLFW_KEY_F14] = Key::F14;
        table[GLFW_KEY_F15] = Key::F15;
        table[GLFW_KEY_F16] = Key::F16;

        table[GLFW_KEY_INSERT] = Key::Insert;
        table[GLFW_KEY_DELETE] = Key::Delete;
        table[GLFW_KEY_HOME] = Key::Home;
        table[GLFW_KEY_END] = Key::End;
        table[GLFW_KEY_PAGE_UP] = Key::PageUp;
        table[GLFW_KEY_PAGE_DOWN] = Key::PageDown;

        table[GLFW_KEY_UP] = Key::Up;
        table[GLFW_KEY_DOWN] = Key::Down;
        table[GLFW_KEY_LEFT] = Key::Left;
        table[GLFW_KEY_RIGHT] = Key::Right;

        table[GLFW_KEY_KP_1] = Key::Numpad1;
        table[GLFW_KEY_KP_2] = Key::Numpad2;
        table[GLFW_KEY_KP_3] = Key::Numpad3;
        table[GLFW_KEY_KP_4] = Key::Numpad4;
        table[GLFW_KEY_KP_5] = Key::Numpad5;
        table[GLFW_KEY_KP_6] = Key::Numpad6;
        table[GLFW_KEY_KP_7] = Key::Numpad7;
        table[GLFW_KEY_KP_8] = Key::Numpad8;
        table[GLFW_KEY_KP_9] = Key::Numpad9;
        table[GLFW_KEY_KP_0] = Key::Numpad0;

        table[GLFW_KEY_KP_DECIMAL] = Key::NumpadPeriod;
        table[GLFW_KEY_KP_DIVIDE] = Key::NumpadDivide;
        table[GLFW_KEY_KP_MULTIPLY] = Key::NumpadMultiply;
        table[GLFW_KEY_KP_SUBTRACT] = Key::NumpadSubtract;
        table[GLFW_KEY_KP_ADD] = Key::NumpadAdd;
        table[GLFW_KEY_KP_ENTER] = Key::NumpadEnter;
        table[GLFW_KEY_KP_EQUAL] = Key::NumpadEqual;

        table[GLFW_KEY_NUM_LOCK] = Key::NumLock;
        table[GLFW_KEY_CAPS_LOCK] = Key::CapsLock;
        table[GLFW_KEY_SCROLL_LOCK] = Key::ScrollLock;

        table[GLFW_KEY_PRINT_SCREEN] = Key::PrintScreen;
        table[GLFW_KEY_PAUSE] = Key::Pause;

        return table;
    }

    constexpr std::array<Key, GLFW_KEYMAP_SIZE> glfwKeyTokenMap = buildGlfwKeyTokenMap();
}

void InputState::endFrame()
{
    keyStatePreviousFrame = keyStateCurrentFrame;
    mouseStatePreviousFrame = mouseStateCurrentFrame;
    mouseDelta = vec2(0,0);
}

void InputState::handleCursorPosUpdate(vec2 newPos)
{
    lastMousePos = currentMousePos;
    currentMousePos = newPos;
    mouseDelta = currentMousePos - lastMousePos;
}

void InputState::handleKeyAction(Key key, ButtonAction action)
{
    keyStateCurrentFrame[static_cast<size_t>(key)] = action == ButtonAction::Press;
}

void InputState::handleMouseAction(MouseButton button, ButtonAction action)
{
    mouseStateCurrentFrame[static_cast<size_t>(button)] = action == ButtonAction::Press;
}

bool InputState::isKeyPressed(Key key, int modifiers) const
{
    // TODO implement modifiers
    const size_t i = static_cast<size_t>(key);
    return keyStateCurrentFrame[i] && !keyStatePreviousFrame[i];
}

bool InputState::isKeyPressed(Key key) const
{
    const size_t i = static_cast<size_t>(key);
    return keyStateCurrentFrame[i] && !keyStatePreviousFrame[i];
}

bool InputState::isKeyReleased(Key key) const
{
    const size_t i = static_cast<size_t>(key);
    return !keyStateCurrentFrame[i] && keyStatePreviousFrame[i];
}

bool InputState::isKeyDown(Key key, int modifiers) const
{
    // TODO implement modifiers
    return keyStateCurrentFrame[static_cast<size_t>(key)];
}

bool InputState::isKeyDown(Key key) const
{
    return keyStateCurrentFrame[static_cast<size_t>(key)];
}

bool InputState::isMousePressed(MouseButton button) const
{
    const size_t i = static_cast<size_t>(button);
    return mouseStateCurrentFrame[i] && !mouseStatePreviousFrame[i];
}

bool InputState::isMouseReleased(MouseButton button) const
{
    const size_t i = static_cast<size_t>(button);
    return !mouseStateCurrentFrame[i] && mouseStatePreviousFrame[i];
}

bool InputState::isMouseDown(MouseButton button) const
{
    return mouseStateCurrentFrame[static_cast<size_t>(button)];
}

void core::glfw_cursorPosCallback(GLFWwindow* window, double x, double y)
{
    InputState* input = static_cast<GlfwApplicationOwnerContext*>(glfwGetWindowUserPointer(window))->input;
    if (input != nullptr)
    {
        input->handleCursorPosUpdate(vec2(static_cast<float>(x), static_cast<float>(y)));
    }
#ifdef DEBUG_BUILD
    // TODO replace with runtime assert
    else
    {
        spdlog::error("Mouse pos input callback was called on a window with a nullptr input manager");
    }
#endif
}

void core::glfw_keyCallback(GLFWwindow* window, int token, int scancode, int action, int mods)
{
    if (action == GLFW_REPEAT)
    {
        // TODO Are you sure this shouldn't be forwarded to the input abstraction first?
        // TODO warning: before you change this if statement, see other warning
        return;
    }

    InputState* input = static_cast<GlfwApplicationOwnerContext*>(glfwGetWindowUserPointer(window))->input;

    if (input != nullptr)
    {
        // TODO map scancodes instead of keys?
        const Key key = glfwKeyTokenMap[token];
        if (key != Key::Unknown) // as long as we can't map the action to a meaningful key, there should be no state change
        {
            // TODO warning: we can only assume action is either press or release because earlier we catch and return if
            //  action = repeat. This could cause a problem if that ever changes.
            input->handleKeyAction(key, action == GLFW_PRESS ? ButtonAction::Press : ButtonAction::Release);
        }
    }
#ifdef DEBUG_BUILD
    // TODO replace with runtime assert
    else
    {
        spdlog::error("Key input callback was called on a window with a nullptr input manager");
    }
#endif
}

void core::glfw_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    InputState* input = static_cast<GlfwApplicationOwnerContext*>(glfwGetWindowUserPointer(window))->input;
    if (input != nullptr)
    {
        // GLFW maps values for buttons [0-7] to Mouse[1-8], though this is no guarantee
        // There is little reason to believe it would ever change though, so let's just static_cast to MouseButton...
        input->handleMouseAction(
            static_cast<MouseButton>(button),
            action == GLFW_PRESS ? ButtonAction::Press : ButtonAction::Release
        );
    }
}
