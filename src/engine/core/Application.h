#pragma once

#include <flecs.h>
#include <string_view>

#include "rendering/Renderer.h"

class GLFWwindow;

namespace core
{
    struct InputState
    {
        int mousePosX, mousePosY;
    };

    struct WindowState
    {
        WindowState() = default;

        WindowState(int width, int height, std::string_view title, bool fullscreen)
            : width(width), height(height),
              fbWidth(width), fbHeight(height),
              title(title),
              fullscreen(fullscreen) { }

        int width, height;
        int fbWidth, fbHeight;
        std::string title;
        bool fullscreen;

        float getFrameBufferAspect() const;
    };

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
        GLFWwindow* _windowPtr = nullptr;
    };
}
