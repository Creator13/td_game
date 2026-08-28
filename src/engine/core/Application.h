#pragma once

#include <flecs.h>
#include <string_view>

#include "assets/AssetDatabase.h"
#include "core/Debug.h"
#include "core/GlfwApplicationOwnerContext.h"
#include "core/Input.h"
#include "core/Window.h"
#include "rendering/Renderer.h"

class GLFWwindow;

namespace core
{
    namespace debug {
        class DebugRenderer;
    }

    class Application
    {
    public: // creation
        Application() = delete;
        Application(int argc, char* argv[], const WindowState& windowState);
        ~Application();

        // No copy (every application is encapsulated and can not exist twice)
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        // Trivial move
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

    public:
        int run();
        flecs::world& getEcsWorld() { return _ecs; }
        const GLFWwindow* getWindowPtr() const { return _windowPtr; }

    private:
        bool createWindow(const WindowState& windowState);
        void initFlecs();
        void cleanup();
        void cleanWindow();

    private: // data
        flecs::world _ecs;
        InputState _inputState;
        WindowState _windowState;

        std::unique_ptr<debug::DebugRenderer> _debugRenderer = nullptr;
        std::unique_ptr<gfx::Renderer> _renderer = nullptr;

        GlfwApplicationOwnerContext _glfwOwnerContext;
        GLFWwindow* _windowPtr = nullptr;
    };
}