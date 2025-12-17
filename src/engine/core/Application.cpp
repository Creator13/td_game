#include "Application.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "core/EcsCore.h"
#include "rendering/EcsRendering.h"
#include "rendering/Renderer.h"

using namespace core;

namespace
{
    void glfw_errorCallback(int code, const char* description)
    {
        spdlog::error("GLFW Error {}: {}", code, description);
    }

    void glfw_framebufferSizeCallback(GLFWwindow* _window, int width, int height)
    {
        WindowState* window = static_cast<GlfwApplicationOwnerContext*>(glfwGetWindowUserPointer(_window))->window;
        if (window != nullptr)
        {
            window->setSize(width, height, true);
        }
        glViewport(0, 0, width, height);
    }
}

void WindowState::setSize(int newWidth, int newHeight, bool setFrameBuffer)
{
    width = newWidth;
    height = newHeight;

    if (setFrameBuffer)
    {
        fbWidth = newWidth;
        fbHeight = newHeight;
    }
}

float WindowState::getFrameBufferAspect() const
{
    return static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
}

Application::Application(int argc, char* argv[], std::string_view resourceRoot, const WindowState& windowState)
    : _ecs(argc, argv), _windowState(windowState)
{
    spdlog::set_level(spdlog::level::debug);

    createWindow(windowState);
    initFlecs();

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Asset database relies on an opengl context and cannot be created before opengl is initialized (createWindow initializes opengl)
    _assetDb = std::make_unique<assets::AssetDatabase>(resourceRoot);
    _renderer.setAssetDatabase(_assetDb.get());
}

Application::~Application()
{
    cleanup();
}

int Application::run()
{
    while (!glfwWindowShouldClose(_windowPtr))
    {
        glfwPollEvents();

        if (!_ecs.progress())
        {
            glfwSetWindowShouldClose(_windowPtr, GLFW_TRUE);
        }

        _renderer.render();

        glfwSwapBuffers(_windowPtr);

        currentFrame++;
        _inputState.endFrame();
    }

    return 0;
}

bool Application::createWindow(const WindowState& windowState)
{
    if (!glfwInit())
    {
        spdlog::error("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _windowPtr = glfwCreateWindow(windowState.width, windowState.height, windowState.title.c_str(), nullptr, nullptr);
    if (!_windowPtr)
    {
        spdlog::error("Failed to create GLFW window.");
        glfwTerminate();
    }

    glfwMakeContextCurrent(_windowPtr);
    glfwSetFramebufferSizeCallback(_windowPtr, glfw_framebufferSizeCallback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        spdlog::error("Failed to initialize GLAD");
        return false;
    }

    glfwSetErrorCallback(glfw_errorCallback);

    _glfwOwnerContext = {
        &_inputState, &_windowState
    };
    glfwSetWindowUserPointer(_windowPtr, &_glfwOwnerContext);

    glfwSetCursorPosCallback(_windowPtr, glfw_cursorPosCallback);
    glfwSetKeyCallback(_windowPtr, glfw_keyCallback);
    glfwSetMouseButtonCallback(_windowPtr, glfw_mouseButtonCallback);

    return true;
}

void Application::initFlecs()
{
#ifdef DEBUG_BUILD
    _ecs.import<flecs::stats>();
    _ecs.set<flecs::Rest>({ });
    spdlog::debug("Open flecs explorer at https://flecs.dev/explorer");
#endif

#ifdef FLECS_LOG
    flecs::log::enable_colors(false);
#endif

    _ecs.import<ecs::engine_core>();
    _ecs.import<ecs::rendering>();

    _ecs.component<ecs::WindowSingleton>().add(flecs::Singleton);

    _ecs.set<ecs::WindowSingleton>({&_windowState});
    _ecs.set<ecs::RendererSingleton>({&_renderer});
    _ecs.set<ecs::GlobalInput>({&_inputState});
}

void Application::cleanup()
{
    cleanWindow();
}

void Application::cleanWindow()
{
    glfwDestroyWindow(_windowPtr);
    glfwTerminate();
}
