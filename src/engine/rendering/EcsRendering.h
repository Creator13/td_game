#pragma once

#include "Mesh.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetId.h"
#include "assets/Shader.h"
#include "core/Transform.h"
#include "math/geom.h"
#include "math/mat4.h"
#include "rendering/Color.h"
#include "rendering/Material.h"
#include "rendering/Renderer.h"

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
        graphics::Color clearColor = graphics::Color::blue;
        // graphics::Color clearColor = graphics::Color(156/255.f, 112/255.f, 139/255.f);
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
    };

    struct MeshRenderData
    {
        assets::AssetRef<Mesh> mesh;
        Material* material;
        CullReason cullReason;
    };

    struct MaterialData
    {
        graphics::Color color;
        // idk what to put here lol
    };

    struct BoxBoundsData
    {
        math::AABB localBounds;
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
        static void updateActivePerspectiveCamera(const PerspectiveCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, CameraRenderData& renderData);
        static void updateActiveOrthoCamera(const OrthoCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, CameraRenderData& renderData);
        static bool submitRenderable(const RendererSingleton& renderer, const HierarchyTransform& transform, const MeshRenderData& renderData);
    };
}
