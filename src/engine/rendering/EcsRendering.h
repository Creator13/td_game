#pragma once

#include "assets/AssetDatabase.h"
#include "math/mat4.h"
#include "rendering/Color.h"

namespace core {
    struct WindowState;
}

namespace graphics
{
    class Renderer;
}

namespace flecs
{
    struct world;
}

namespace core::ecs
{
    // #### CAMERA ####
    struct PerspectiveCameraData
    {
        float fov;
        float near, far;
    };

    struct OrthoCameraData
    {
        float orthoSize;
        float near, far;
    };

    struct CameraRenderData
    {
        graphics::Color clearColor = graphics::Color::lightBlue;
        math::mat4 projectionMatrix = math::mat4::identity;
        math::mat4 viewMatrix = math::mat4::identity;
    };

    struct ActiveCamera { };

    // #### RENDER COMPONENTS ####
    struct MeshRenderer
    {
        assets::AssetId meshId;
        assets::AssetId shaderId;
    };

    struct MaterialData
    {
        graphics::Color color;
        // idk what to put here lol
    };

    // #### SINGLETON ####
    struct WindowSingleton
    {
        const WindowState* windowState;
    };

    struct RendererSingleton
    {
        graphics::Renderer* renderer;
    };

    struct rendering
    {
        rendering(flecs::world& ecs);
    };
}
