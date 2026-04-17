#include "rendering/EcsRendering.h"

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include "core/Constants.h"
#include "core/Transform.h"
#include "core/Window.h"
#include "math/geom.h"
#include "rendering/Pipeline.h"
#include "rendering/Renderer.h"

using namespace math;
using namespace core::ecs;
using namespace core::gfx;

namespace
{
    constexpr bool isAABBBehindPlane(const AABB& aabb, const plane& plane)
    {
        const float r = dot(comptAbs(plane.normal), aabb.halfExtents);
        const float dist = dot(plane.normal, aabb.center) + plane.dist;
        return dist < -r;
    }

    constexpr bool isAABBInFrustum(const AABB& aabb, const frustum& frustum)
    {
        if (isAABBBehindPlane(aabb, frustum.near)) return false;
        if (isAABBBehindPlane(aabb, frustum.far)) return false;
        if (isAABBBehindPlane(aabb, frustum.left)) return false;
        if (isAABBBehindPlane(aabb, frustum.right)) return false;
        if (isAABBBehindPlane(aabb, frustum.top)) return false;
        if (isAABBBehindPlane(aabb, frustum.bottom)) return false;

        return true;
    }

    void registerComponents(flecs::world& ecs)
    {
        ecs.component<ActiveCamera>();

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

        ecs.component<MeshRenderData>();
        // TODO minimal reflection data

        ecs.component<ViewportData>().add(flecs::Singleton);
        ecs.component<WindowSingleton>().add(flecs::Singleton);
        ecs.component<RendererSingleton>().add(flecs::Singleton);
    }

    // TODO find a solution for this that I love more (CurrentActiveCamera with an entity target?)
    flecs::entity currentActiveCameraEntity;
}

rendering::rendering(flecs::world& ecs)
{
    ecs.module<rendering>("Rendering");

    registerComponents(ecs);

    ecs.set<ViewportData>({ });

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

    ecs.observer<const MeshRenderData>("Bounding volume matcher observer")
        .event(flecs::OnSet)
        .each([](flecs::entity e, const MeshRenderData& renderData)
        {
            // TODO find a mechanism that verifies whether actual mesh reference changes and modify bounds accordingly
            //  Could be as simple as wrapper methods that all call e.modified()
            const AABB& bounds = renderData.mesh->bounds;
            e.set<BoxBoundsData>({bounds});
        });

    auto perspSystem = ecs.system<const PerspectiveCameraData, const HierarchyTransform, const WindowSingleton, ViewportData>("Perspective camera update system")
        .kind(flecs::PreStore)
        .with<ActiveCamera>()
        .each(updateActivePerspectiveCamera);

    auto orthoSystem = ecs.system<const OrthoCameraData, const HierarchyTransform, const WindowSingleton, ViewportData>("Ortho camera update system")
        .kind(flecs::PreStore)
        .with<ActiveCamera>()
        .each(updateActiveOrthoCamera);

    ecs.system<const RendererSingleton, const ViewportData>()
        .each(syncRendererToActiveCamera)
        .depends_on(perspSystem)
        .depends_on(orthoSystem);

    auto cullingSystem = ecs.system<const HierarchyTransform, const BoxBoundsData, MeshRenderData>("Culling system")
        .multi_threaded()
        .run([](flecs::iter& it)
        {
            ZoneScopedN("_engine::CullingSystem");

            auto& camera = it.world().get<const ViewportData>();
            const frustum frustum = frustum::fromViewProjectionMatrix(camera.projectionMatrix * constants::COORDINATE_BASIS * camera.viewMatrix);

            while (it.next())
            {
                auto f_transform = it.field<const HierarchyTransform>(0);
                auto f_bounds = it.field<const BoxBoundsData>(1);
                auto f_renderer = it.field<MeshRenderData>(2);

                for (auto i : it)
                {
                    const HierarchyTransform& transform = f_transform[i];
                    const BoxBoundsData& bounds = f_bounds[i];
                    MeshRenderData& renderer = f_renderer[i];

                    renderer.cullReason = CullReason::None;

                    const mat4& worldMat = transform.getWorldMatrix();

                    // TODO caching world bounds is a decently easy optimization
                    AABB worldBounds;
                    worldBounds.center = (worldMat * vec4(bounds.localBounds.center, 1.0f)).xyz();
                    worldBounds.halfExtents.x =
                        math::abs(worldMat.get(0, 0)) * bounds.localBounds.halfExtents.x +
                        math::abs(worldMat.get(0, 1)) * bounds.localBounds.halfExtents.y +
                        math::abs(worldMat.get(0, 2)) * bounds.localBounds.halfExtents.z;

                    worldBounds.halfExtents.y =
                        math::abs(worldMat.get(1, 0)) * bounds.localBounds.halfExtents.x +
                        math::abs(worldMat.get(1, 1)) * bounds.localBounds.halfExtents.y +
                        math::abs(worldMat.get(1, 2)) * bounds.localBounds.halfExtents.z;

                    worldBounds.halfExtents.z =
                        math::abs(worldMat.get(2, 0)) * bounds.localBounds.halfExtents.x +
                        math::abs(worldMat.get(2, 1)) * bounds.localBounds.halfExtents.y +
                        math::abs(worldMat.get(2, 2)) * bounds.localBounds.halfExtents.z;

                    // Cull entity if it falls outside the frustum
                    if (!isAABBInFrustum(worldBounds, frustum))
                    {
                        renderer.cullReason = CullReason::Frustum;
                    }
                }
            }
        });

    ecs.system<const HierarchyTransform, const MeshRenderData>("Scene geometry collection")
        // .multi_threaded() // TODO make multithreaded (but obv can't while renderer doesn't have a thread-safe render list)
        .kind(flecs::OnStore)
        .run([](flecs::iter& it)
        {
            ZoneScopedN("Scene geometry collection system");

            const auto& renderer = it.world().get<const RendererSingleton>();

            int total = 0;
            int rendered = 0;

            while (it.next())
            {
                auto f_transform = it.field<const HierarchyTransform>(0);
                auto f_renderData = it.field<const MeshRenderData>(1);

                for (const auto i : it)
                {
                    const auto& renderData = f_renderData[i];
                    total++;

                    // Exclude culled entities
                    if (renderData.cullReason != CullReason::None) continue;

                    // Submit command
                    DrawCommand command;
                    command.sortKey = Renderer::buildSortKey(renderData.material, renderData.mesh);
                    command.mesh = renderData.mesh->gpuHandle;
                    command.material = renderData.material;
                    command.modelMatrix = f_transform[i].getWorldMatrix();
                    command.queue = DrawCommand::RenderQueue::OPAQUE;

                    renderer.ptr->submitDrawCommand(command);
                    rendered++;
                }
            }
        })
        .depends_on(cullingSystem);
}

void rendering::syncRendererToActiveCamera(const RendererSingleton& r_ptr, const ViewportData& renderData)
{
    Renderer& renderer = *r_ptr.ptr;
    renderer.setViewportData(renderData);
}

void rendering::updateActivePerspectiveCamera(const PerspectiveCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, ViewportData& renderData)
{
    float aspect = window.state->getFrameBufferAspect();

    renderData.projectionMatrix = mat4::makePerspective(cameraData.fov, aspect, cameraData.near, cameraData.far);
    renderData.viewMatrix = inverse(transform.getWorldMatrix());

    renderData.pixelWidth = window.state->fbWidth;
    renderData.pixelHeight = window.state->fbHeight;
}

void rendering::updateActiveOrthoCamera(const OrthoCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, ViewportData& renderData)
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
    renderData.viewMatrix = inverse(transform.getWorldMatrix());

    renderData.pixelWidth = window.state->fbWidth;
    renderData.pixelHeight = window.state->fbHeight;
}
