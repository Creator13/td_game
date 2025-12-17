#pragma once

#include <string>

class GLFWwindow;

namespace core
{
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

        void setSize(int newWidth, int newHeight, bool setFrameBuffer = false);
        float getFrameBufferAspect() const;
    };

    void glfw_framebufferSizeCallback(GLFWwindow* window, int width, int height);
}
