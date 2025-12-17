#include "core/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/GlfwApplicationOwnerContext.h"

using namespace core;

void WindowState::setSize(int newWidth, int newHeight, bool setFrameBuffer)
{
    width = newWidth;
    height = newHeight;

    if (setFrameBuffer)
    {
        fbWidth = newWidth;
        fbHeight = newHeight;
    }

    glViewport(0, 0, width, height);
}

float WindowState::getFrameBufferAspect() const
{
    return static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
}

void core::glfw_framebufferSizeCallback(GLFWwindow* _window, int width, int height)
{
    WindowState* window = static_cast<GlfwApplicationOwnerContext*>(glfwGetWindowUserPointer(_window))->window;
    if (window != nullptr)
    {
        window->setSize(width, height, true);
    }
}
