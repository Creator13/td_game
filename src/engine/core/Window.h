#pragma once

#include <string>

#include "datatype.h"

class GLFWwindow;

namespace core
{
    struct WindowState
    {
        enum class FullscreenMode { Windowed, Borderless, Exclusive};

        WindowState() = default;

        WindowState(int width, int height, std::string_view title, FullscreenMode fullscreenMode)
            : width(width), height(height),
              fbWidth(width), fbHeight(height),
              title(title),
              fullscreenMode(fullscreenMode) { }

        u16 width, height;
        u16 fbWidth, fbHeight;
        std::string title;
        FullscreenMode fullscreenMode;

        void setSize(int newWidth, int newHeight, bool setFrameBuffer = true);
        float getFrameBufferAspect() const;
    };

    void glfw_framebufferSizeCallback(GLFWwindow* window, int width, int height);
}
