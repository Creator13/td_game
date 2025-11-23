#include "Application.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "core/EcsCore.h"
#include "rendering/EcsRendering.h"
#include "rendering/Renderer.h"

using namespace core;

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
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
    glfwSetFramebufferSizeCallback(_windowPtr, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        spdlog::error("Failed to initialize GLAD");
        return false;
    }

    return true;
}

void Application::initFlecs()
{
#ifdef DEBUG_BUILD
    _ecs.import<flecs::stats>();
    _ecs.set<flecs::Rest>({ });
    spdlog::debug("Open flecs explorer at https://flecs.dev/explorer");
#endif

    _ecs.import<ecs::engine_core>();
    _ecs.import<ecs::rendering>();

    _ecs.component<ecs::WindowSingleton>().add(flecs::Singleton);
    _ecs.set<ecs::WindowSingleton>({&_windowState});
    _ecs.set<ecs::RendererSingleton>({&_renderer});
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
