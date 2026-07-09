#pragma once

#include "datatype.h"
#include "assets/AssetId.h"
#include "assets/Mesh.h"
#include "core/Transform.h"
#include "math/geom.h"
#include "rendering/Material.h"
#include "rendering/Renderer.h"

namespace core
{
    struct WindowState;
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
        assets::AssetRef<Material> material;
        CullReason cullReason;
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
        gfx::Renderer* ptr;
    };

    struct SceneRenderData
    {
        std::vector<assets::AssetRef<Material>> postEffects;
    };

    struct rendering
    {
        explicit rendering(flecs::world& ecs);

    private:
        static void syncRendererToActiveCamera(const RendererSingleton& r_ptr, const gfx::ViewportData& viewportData);
        static void updateActivePerspectiveCamera(const PerspectiveCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, gfx::ViewportData& viewportData);
        static void updateActiveOrthoCamera(const OrthoCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, gfx::ViewportData& viewportData);
        static void syncPostEffectStack(const RendererSingleton& r_ptr, const SceneRenderData& sceneRenderData);
    };
}
