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

std::optional<std::unique_ptr<Application>> Application::init(std::string_view resourceRoot, const WindowState& windowState)
{
    spdlog::set_level(spdlog::level::debug);

    std::unique_ptr<Application> app(new Application());
    if (!app->createWindow(windowState))
    {
        return { };
    }

    res::initResources(resourceRoot);

    return app;
}

Application::~Application()
{
    cleanup();
}

int Application::run()
{
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    math::vec3 vertices[] = {
        math::vec3(0, 0, 0) + math::vec3::one * -.5f,
        math::vec3(0, 0, 1) + math::vec3::one * -.5f,
        math::vec3(1, 0, 1) + math::vec3::one * -.5f,
        math::vec3(1, 0, 0) + math::vec3::one * -.5f,
        math::vec3(0, 1, 0) + math::vec3::one * -.5f,
        math::vec3(0, 1, 1) + math::vec3::one * -.5f,
        math::vec3(1, 1, 1) + math::vec3::one * -.5f,
        math::vec3(1, 1, 0) + math::vec3::one * -.5f,
    };
    unsigned int indices[] = {
        0, 3, 2, // front
        0, 2, 1,
        3, 7, 6, // right
        3, 6, 2,
        7, 4, 5, // back
        7, 5, 6,
        4, 0, 1, // left
        4, 1, 5,
        1, 2, 6, // top
        1, 6, 5,
        4, 7, 3, // bottom
        4, 3, 0
    };

    GLuint vbo, vao, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(math::vec3) * 8, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    graphics::GpuMesh meshHandle{vao, vbo, ebo, 36};

    graphics::shader::ShaderProgramData shader = res::loadShader(
        fs::path("shaders/basic.vert"),
        fs::path("shaders/basic.frag")
    );

    graphics::shader::ShaderProgramData colorShader = res::loadShader(
        fs::path("shaders/basic.vert"),
        fs::path("shaders/color.frag"));

    math::vec3 camPos = math::vec3(1, -4, 2);
    math::rot3x3 camRot = math::rot3x3::eulerAngles(0, 0, 25);
    math::mat4 worldToView = inverse(math::mat4::makeTRS(camPos, camRot, math::vec3::one));

    math::mat4 coordinateBasis = math::mat4(
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, -1, 0, 0,
        0, 0, 0, 1
    );
    worldToView = coordinateBasis * worldToView;
    math::mat4 viewToClip = math::mat4::makePerspective(75, (float)_windowState.width / (float)_windowState.height, .1f, 100.f);

    graphics::Renderer renderer;
    renderer.setClearColor(graphics::color(.4, 0.4, 0.4, 0));
    renderer.setVpMatrix(viewToClip * worldToView);

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
            math::vec3(.01, .01, 10)
        );

        renderer.submit(graphics::Renderable{localToWorld, meshHandle, shader, {{0, 0, 0, 0}}});
        renderer.submit(graphics::Renderable{xAxis, meshHandle, colorShader, {{1, 0, 0, 1}}});
        renderer.submit(graphics::Renderable{yAxis, meshHandle, colorShader, {{0, 1, 0, 1}}});
        renderer.submit(graphics::Renderable{zAxis, meshHandle, colorShader, {{0, 0, 1, 1}}});
        renderer.render();

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
