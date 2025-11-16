#pragma once

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
        WindowState(int width, int height, const char* title, bool fullscreen)
            : width(width), height(height),
              fbWidth(width), fbHeight(height),
              title(title),
              fullscreen(fullscreen) { }

        int width, height;
        int fbWidth, fbHeight;
        const char* title;
        bool fullscreen;
    };

    class Application
    {
    public: // creation
        Application() = delete;
        Application(std::string_view resourceRoot, const WindowState& windowState);
        ~Application();

        // No copy (every application is encapsulated and can not exist twice)
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        // Trivial move
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

    public:
        int run();

    private:
        bool createWindow(const WindowState& windowState);
        void cleanup();
        void cleanWindow();

    private: // data
        InputState _inputState;
        WindowState _windowState;
        graphics::Renderer _renderer;
        assets::AssetDatabase* _assetDb = nullptr;
        GLFWwindow* _windowPtr = nullptr;
    };
}
