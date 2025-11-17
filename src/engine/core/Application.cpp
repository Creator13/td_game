#include "Application.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "resources.h"
#include "rendering/shader.h"
#include "math/math.h"
#include "rendering/Renderer.h"

using namespace core;

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

Application::Application(std::string_view resourceRoot, const WindowState& windowState)
{
    spdlog::set_level(spdlog::level::debug);

    createWindow(windowState);

    res::initResources(resourceRoot);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    _windowState = windowState;
    _assetDb = new assets::AssetDatabase(resourceRoot);
    _renderer.setAssetDatabase(_assetDb);
    _inputState = InputState();
}

Application::~Application()
{
    cleanup();
}

int Application::run()
{
    math::vec3 camPos = math::vec3(1, -4, 2);
    math::rot3x3 camRot = math::rot3x3::eulerAngles(0, 0, 25);
    math::mat4 worldToView = inverse(math::mat4::makeTRS(camPos, camRot, math::vec3::one));
    math::mat4 viewToClip = math::mat4::makePerspective(75, (float) _windowState.width / (float) _windowState.height, .1f, 100.f);

    _renderer.setClearColor(graphics::color(.4, 0.4, 0.4, 0));
    _renderer.setWorldToViewMatrix(worldToView);
    _renderer.setViewToClipMatrix(viewToClip);

    graphics::shader::ShaderProgramData shader = res::loadShader(
        fs::path("shaders/basic.vert"),
        fs::path("shaders/basic.frag")
    );

    graphics::shader::ShaderProgramData colorShader = res::loadShader(
        fs::path("shaders/basic.vert"),
        fs::path("shaders/color.frag"));

    assets::AssetId cube = assets::AssetDatabase::idFromPath("@internal/mesh/cube");
    assets::AssetId quad = assets::AssetDatabase::idFromPath("@internal/mesh/quad");

    while (!glfwWindowShouldClose(_windowPtr))
    {
        glfwPollEvents();

        math::mat4 localToWorld = math::mat4::makeTRS(
            math::vec3::zero,
            math::rot3x3::eulerAngles(0, 0, glfwGetTime() * 45),
            math::vec3::one);

        math::mat4 xAxis = math::mat4::makeTRS(
            math::vec3::zero,
            math::rot3x3::identity,
            math::vec3(10, .01, .01)
        );
        math::mat4 yAxis = math::mat4::makeTRS(
            math::vec3::zero,
            math::rot3x3::identity,
            math::vec3(.01, 10, .01)
        );
        math::mat4 zAxis = math::mat4::makeTRS(
            math::vec3::zero,
            math::rot3x3::identity,
            math::vec3(.01, .01, 10));

        _renderer.submit({
            localToWorld,
            cube,
            shader, { }
        });

        _renderer.submit({
            xAxis,
            cube,
            colorShader, { graphics::color(1, 0, 0)}
        });

        _renderer.submit({
            yAxis,
            cube,
            colorShader, { graphics::color(0, 1, 0) }
        });

        _renderer.submit({
            zAxis,
            cube,
            colorShader, { graphics::color(0, 0, 1) }
        });

        _renderer.render();

        glfwSwapBuffers(_windowPtr);
    }

    return 0;
}

bool Application::createWindow(const WindowState& windowState)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _windowPtr = glfwCreateWindow(windowState.width, windowState.height, windowState.title, nullptr, nullptr);
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

    _windowState = windowState;

    return true;
}

void Application::cleanup()
{
    res::unloadResources();
    cleanWindow();
}

void Application::cleanWindow()
{
    glfwDestroyWindow(_windowPtr);
    glfwTerminate();
}
