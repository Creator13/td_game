#include "Application.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "resources.h"
#include "rendering/shader.h"
using namespace render;

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

const char* vert_shader =
        "#version 460 core\n"
        "in vec3 vp;"
        "void main() {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";

const char* frag_shader =
        "#version 460 core\n"
        "out vec4 frag_color;"
        "void main() {"
        "  frag_color = vec4(0.941, 0.808, 0.6, 1.0);"
        "}";

int Application::run()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window." << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    float points[] = {
        0.0f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9, points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    shader::ShaderProgramData shader = res::loadShader(
        fs::path("res/shaders/basic.vert"),
        fs::path("res/shaders/basic.frag")
    );

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(.2f, .3f, .4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader::use(shader);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}
