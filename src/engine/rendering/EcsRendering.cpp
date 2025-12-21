#include "rendering/EcsRendering.h"

#include <flecs.h>

#include "core/Transform.h"
#include "core/Window.h"
#include "rendering/Renderer.h"

using namespace math;
using namespace core::ecs;

namespace
{
    void registerComponents(flecs::world& ecs)
    {
        ecs.component<ActiveCamera>();
        ecs.component<CameraRenderData>().add(flecs::Singleton);

        ecs.component<PerspectiveCameraData>("Camera (perspective)")
            .member<float>("Fov").range(5.f, 150.f)
            .member<float>("Near plane")
            .member<float>("Far plane");

        ecs.component<OrthoCameraData>("Camera (orthographic)")
            .member<float>("Orthographic size")
            .member<float>("Near plane")
            .member<float>("Far plane");

        ecs.component<CullReason>("Culling reason")
            .constant("None", CullReason::None)
            .constant("Frustum", CullReason::Frustum)
            .constant("LOD", CullReason::LOD);

        ecs.component<MeshRenderer>()
            .member<uint64_t>("Mesh id")
            .member<uint64_t>("Shader id")
            .member<CullReason>("Culling reason");

        ecs.component<MaterialData>();

        ecs.component<WindowSingleton>().add(flecs::Singleton);
        ecs.component<RendererSingleton>().add(flecs::Singleton);
    }

    constexpr vec3 extractPositionFromTRS(const mat4& trs)
    {
        return trs.getRow(3).xyz();
    }

    // TODO find a solution for this that I love more (CurrentActiveCamera with an entity target?)
    flecs::entity currentActiveCameraEntity;
}

rendering::rendering(flecs::world& ecs)
{
    ecs.module<rendering>("Rendering");

    registerComponents(ecs);

    ecs.set<CameraRenderData>({ });

    ecs.observer("Active camera uniqueness observer")
        .with<ActiveCamera>()
        .event(flecs::OnAdd)
        .each([](flecs::entity e)
        {
            if (currentActiveCameraEntity.is_alive())
            {
                currentActiveCameraEntity.remove<ActiveCamera>();
            }
            currentActiveCameraEntity = e;
        });

    ecs.system<const PerspectiveCameraData, const WorldTransformData, const WindowSingleton, CameraRenderData>("Perspective camera update system")
        .kind(flecs::PreStore)
        .with<ActiveCamera>()
        .each(updateActivePerspectiveCamera);

    ecs.system<const OrthoCameraData, const WorldTransformData, const WindowSingleton, CameraRenderData>("Ortho camera update system")
        .kind(flecs::PreStore)
        .with<ActiveCamera>()
        .each(updateActiveOrthoCamera);

    auto cameraSystem = ecs.system<const RendererSingleton, const CameraRenderData>()
        .kind(flecs::OnStore)
        .each(syncRendererToActiveCamera);

    auto cullingSystem = ecs.system<const WorldTransformData, MeshRenderer, const CameraRenderData>("Culling system")
        .multi_threaded()
        .each([](const WorldTransformData& transform, MeshRenderer rend, const CameraRenderData& camera)
        {
            rend.cullReason = CullReason::None;
        })
        .depends_on(cameraSystem);

    ecs.system<const RendererSingleton, const TransformHandle, const MeshRenderer, const MaterialData>("Render system")
        .each(submitRenderable)
        .depends_on(cameraSystem)
        .depends_on(cullingSystem);
}

void rendering::submitRenderable(const RendererSingleton& renderer, const TransformHandle& transform, const MeshRenderer& renderData, const MaterialData& mat)
{
    graphics::Renderable renderable;
    renderable.meshId = renderData.meshId;
    renderable.shaderId = renderData.shaderId;
    renderable.modelMatrix = transform.worldTransformMatrix;
    renderable.material = {mat.color};

    renderer.ptr->submit(renderable);
}

void rendering::updateActiveOrthoCamera(const OrthoCameraData& cameraData, const WorldTransformData& transform, const WindowSingleton& window, CameraRenderData& renderData)
{
    float aspect = window.state->getFrameBufferAspect();
    renderData.projectionMatrix = mat4::makeOrtho(
        -cameraData.orthoSize * aspect,
        cameraData.orthoSize * aspect,
        -cameraData.orthoSize,
        cameraData.orthoSize,
        cameraData.near,
        cameraData.far
    );
    renderData.viewMatrix = inverse(transform.worldTransformMatrix);
}

void rendering::updateActivePerspectiveCamera(const PerspectiveCameraData& cameraData, const WorldTransformData& transform, const WindowSingleton& window, CameraRenderData& renderData)
{
    float aspect = window.state->getFrameBufferAspect();
    renderData.projectionMatrix = mat4::makePerspective(cameraData.fov, aspect, cameraData.near, cameraData.far);
    renderData.viewMatrix = inverse(transform.worldTransformMatrix);
}

void rendering::syncRendererToActiveCamera(const RendererSingleton& r_ptr, const CameraRenderData& renderData)
{
    graphics::Renderer& renderer = *r_ptr.ptr;
    renderer.setClearColor(renderData.clearColor);
    renderer.setWorldToViewMatrix(renderData.viewMatrix);
    renderer.setViewToClipMatrix(renderData.projectionMatrix);
}
