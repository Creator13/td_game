#pragma once

#include <flecs.h>
#include <string_view>

#include "Transform.h"
#include "core/Window.h"
#include "core/GlfwApplicationOwnerContext.h"
#include "core/Input.h"
#include "rendering/Renderer.h"

class GLFWwindow;

namespace core
{
    class Application
    {
    public: // creation
        Application() = delete;
        Application(int argc, char* argv[], std::string_view resourceRoot, const WindowState& windowState);
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
        assets::AssetDatabase& assets() const { return *_assetDb.get(); }
        // TransformSystem& getTransformSys() const { return *_transformSystem.get(); }
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
        graphics::Renderer _renderer;

        std::unique_ptr<assets::AssetDatabase> _assetDb = nullptr;
        // std::unique_ptr<TransformSystem> _transformSystem = nullptr;

        GlfwApplicationOwnerContext _glfwOwnerContext;
        GLFWwindow* _windowPtr = nullptr;

        uint32_t currentFrame = 0;
    };
}