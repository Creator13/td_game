#pragma once

#include "assets/AssetId.h"
#include "core/Transform.h"
#include "math/mat4.h"
#include "rendering/Color.h"

namespace core
{
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
    struct WorldTransformData;

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
    enum class CullReason
    {
        None = 0,
        Frustum = 1,
        LOD = 2,
        Inherit = 3,
    };

    struct MeshRenderer
    {
        assets::AssetId meshId;
        assets::AssetId shaderId;
        CullReason cullReason;
    };

    struct MaterialData
    {
        graphics::Color color;
        // idk what to put here lol
    };

    // #### SINGLETON ####
    struct WindowSingleton
    {
        const WindowState* state;
    };

    struct RendererSingleton
    {
        graphics::Renderer* ptr;
    };

    struct rendering
    {
        explicit rendering(flecs::world& ecs);

    private:
        static void syncRendererToActiveCamera(const RendererSingleton& r_ptr, const CameraRenderData& renderData);
        static void updateActivePerspectiveCamera(const PerspectiveCameraData& cameraData, const WorldTransformData& transform, const WindowSingleton& window, CameraRenderData& renderData);
        static void updateActiveOrthoCamera(const OrthoCameraData& cameraData, const WorldTransformData& transform, const WindowSingleton& window, CameraRenderData& renderData);
        static void submitRenderable(const RendererSingleton& renderer, const TransformHandle& transform, const MeshRenderer& renderData, const MaterialData& mat);
    };
}
