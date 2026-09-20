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

    constexpr bool isSphereBehindPlane(const sphere& sphere, const plane& plane)
    {
        return dot(plane.normal, sphere.center) + plane.dist < -sphere.radius;
    }

    constexpr bool isSphereInFrustum(const sphere& sphere, const frustum& frustum)
    {
        if (isSphereBehindPlane(sphere, frustum.near)) return false;
        if (isSphereBehindPlane(sphere, frustum.far)) return false;
        if (isSphereBehindPlane(sphere, frustum.left)) return false;
        if (isSphereBehindPlane(sphere, frustum.right)) return false;
        if (isSphereBehindPlane(sphere, frustum.top)) return false;
        if (isSphereBehindPlane(sphere, frustum.bottom)) return false;

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
        ecs.component<SceneRenderData>().add(flecs::Singleton);
        ecs.component<WindowSingleton>().add(flecs::Singleton);
        ecs.component<RendererSingleton>().add(flecs::Singleton);
    }

    constexpr float lightAttenuation(float squareDistance, float lightRange)
    {
        const float squareRange = lightRange * lightRange;
        const float factor = clamp01(1.0f - (squareDistance * squareDistance) / (squareRange * squareRange));
        const float window = factor * factor;
        const float invSquare = 1.0f / max(squareDistance, 0.0001f);
        return invSquare * window;
    }

    int sortLightsComparison(flecs::entity_t e1, const LightData* l1, flecs::entity_t e2, const LightData* l2)
    {
        (void) e1;
        (void) e2;

        if (l1->type != l2->type)
        {
            return static_cast<int>(l1->type) - static_cast<int>(l2->type);
        }

        return (l1->score > l2->score) - (l1->score < l2->score);
    }

    void scoreLights(LightData& light, const HierarchyTransform& transform, const ViewportData& viewportData)
    {
        switch (light.type)
        {
            case LightData::Type::Directional:
            {
                light.score = light.intensity;
                break;
            }
            case LightData::Type::Point:
            {
                const float sqrDist = sqrDistance(transform.getWorldPosition(), viewportData.cameraPos);
                light.score = light.intensity * lightAttenuation(sqrDist, light.range);
                break;
            }
            case LightData::Type::Spot:
            {
                const vec3 toCamera = viewportData.cameraPos - transform.getWorldPosition();
                const float squareDistance = toCamera.sqrLength();
                const vec3 dirToCamera = toCamera * (1.0f / math::sqrt(max(squareDistance, 0.0001f)));

                const float innerCutoff = math::cos(light.cutoffDegrees * .5f * .5f * DEG2RAD);
                const float outerCutoff = math::cos(light.cutoffDegrees * .5f * DEG2RAD);
                const float theta = dot(transform.getForward(), dirToCamera);

                const float distAttenuation = lightAttenuation(squareDistance, light.range);
                const float coneAttenuation = smoothstep(outerCutoff, innerCutoff, theta);

                light.score = light.intensity * distAttenuation * coneAttenuation;
                break;
            }
            default:
                light.score = light.intensity;
                break;
        }
    }

    void collectLights(flecs::iter& it)
        {
            const auto& renderer = it.world().get<const RendererSingleton>();

            const auto& camera = it.world().get<const ViewportData>();
            const frustum camFrustum = frustum::fromViewProjectionMatrix(camera.projectionMatrix * constants::COORDINATE_BASIS * camera.viewMatrix);

            bool hasMainLight = false;
            int numPointLights = 0;
            int numDirectionalLights = 0;
            int numSpotlights = 0;

            while (it.next())
            {
                auto f_lightData = it.field<const LightData>(0);
                auto f_transform = it.field<const HierarchyTransform>(1);

                for (auto i : it)
                {
                    const HierarchyTransform& transform = f_transform[i];
                    const LightData& ecsLight = f_lightData[i];

                    switch (ecsLight.type)
                    {
                        case LightData::Type::Directional:
                        {
                            // Culling: limit to MAX_LIGHTS, but also do not submit any lights with a score of 0 (no contribution)
                            if (numDirectionalLights >= LightingDataBlock::MAX_LIGHTS) break;
                            if (ecsLight.score <= 0) break;

                            LightingDataBlock::DirectionalLight dirLight{
                                .direction = transform.getForward(),
                                .color = ecsLight.color.rgbVec3(),
                                .intensity = ecsLight.intensity
                            };
                            if (hasMainLight)
                            {
                                // TODO set the first encountered light as the main light
                                // renderer.ptr->setMainLight(...)
                                hasMainLight = true;
                            }
                            renderer.ptr->submitDirectionalLight(dirLight);
                            numDirectionalLights++;
                            break;
                        }
                        case LightData::Type::Point:
                        {
                            // Culling: limit to MAX_LIGHTS
                            if (numPointLights >= LightingDataBlock::MAX_LIGHTS) break;
                            vec3 position = transform.getWorldPosition();
                            if (!isSphereInFrustum({.center = position, .radius = ecsLight.range}, camFrustum)) break;
                            if (ecsLight.score <= 0) break;

                            LightingDataBlock::PointLight pointLight{
                                .position = position,
                                .color = ecsLight.color.rgbVec3(),
                                .intensity = ecsLight.intensity,
                                .range = ecsLight.range
                            };
                            renderer.ptr->submitPointLight(pointLight);
                            numPointLights++;
                            break;
                        }
                        case LightData::Type::Spot:
                        {
                            if (numPointLights >= LightingDataBlock::MAX_LIGHTS) break;

                            LightingDataBlock::Spotlight spotlight{
                                .position = transform.getWorldPosition(),
                                .direction = transform.getForward(),
                                .innerCutoff = math::cos(ecsLight.cutoffDegrees * .5f * .5f * DEG2RAD), // Hardcode half the outer width, improve once there are better data structures
                                .color = ecsLight.color.rgbVec3(),
                                .outerCutoff = math::cos(ecsLight.cutoffDegrees * .5f * DEG2RAD),
                                .intensity = ecsLight.intensity,
                                .range = ecsLight.range,
                            };
                            renderer.ptr->submitSpotlight(spotlight);
                            numSpotlights++;
                            break;
                        }
                        default:
                            ENGINE_ASSERT(false, "Invlaid");
                            break;
                    }
                }
            }
        }

    // TODO find a solution for this that I love more (CurrentActiveCamera with an entity target?)
    flecs::entity currentActiveCameraEntity;
}

rendering::rendering(flecs::world& ecs)
{
    ecs.module<rendering>("Rendering");

    registerComponents(ecs);

    ecs.set<ViewportData>({ });
    ecs.set<SceneRenderData>({ });

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

    auto viewportSyncSystem = ecs.system<const RendererSingleton, const ViewportData>()
        .each(syncRendererToActiveCamera)
        .depends_on(perspSystem)
        .depends_on(orthoSystem);

    ecs.system<const RendererSingleton, const SceneRenderData>("Scene data synchronization system")
        .kind(flecs::PreStore)
        .each(syncSceneData);

    auto lightScoringSystem = ecs.system<LightData, const HierarchyTransform, const ViewportData>("Light scoring system")
        .each(scoreLights)
        .depends_on(viewportSyncSystem);

    ecs.system<const LightData, const HierarchyTransform>("Light collection system")
        .order_by(sortLightsComparison)
        .run(collectLights)
        .depends_on(lightScoringSystem);

    auto cullingSystem = ecs.system<const HierarchyTransform, const BoxBoundsData, MeshRenderData>("Culling system")
        .kind(flecs::OnStore)
        .multi_threaded()
        .run([](flecs::iter& it)
        {
            ZoneScopedN("Culling system");

            auto& camera = it.world().get<const ViewportData>();
            const frustum frustum = frustum::fromViewProjectionMatrix(camera.projectionMatrix * constants::COORDINATE_BASIS * camera.viewMatrix);

            while (it.next())
            {
                auto f_transform = it.field<const HierarchyTransform>(0);
                auto f_bounds = it.field<const BoxBoundsData>(1);
                auto f_renderData = it.field<MeshRenderData>(2);

                for (auto i : it)
                {
                    const HierarchyTransform& transform = f_transform[i];
                    const BoxBoundsData& bounds = f_bounds[i];
                    MeshRenderData& renderData = f_renderData[i];

                    renderData.cullReason = CullReason::None;

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
                        renderData.cullReason = CullReason::Frustum;
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
                    command.instanceData.transform = f_transform[i].getWorldMatrix();
                    command.instanceData.invTransform = inverse(command.instanceData.transform); // TODO cache inverse matrix on objects, this is expensive to calculate each frame.
                    command.queue = DrawCommand::RenderQueue::OPAQUE;
                    renderer.ptr->submitDrawCommand(command);

                    if (renderData.castShadow)
                    {
                        // TODO implement shadow casting of object as override of pipeline setting
                        renderer.ptr->submitShadowCommand(renderData.mesh, f_transform[i].getWorldMatrix());
                    }
                    rendered++;
                }
            }
        })
        .depends_on(cullingSystem);
}

void rendering::syncRendererToActiveCamera(const RendererSingleton& r_ptr, const ViewportData& viewportData)
{
    Renderer& renderer = *r_ptr.ptr;
    renderer.setViewportData(viewportData);
}

void rendering::syncSceneData(const RendererSingleton& r_ptr, const SceneRenderData& sceneRenderData)
{
    Renderer& renderer = *r_ptr.ptr;
    renderer.setPostEffectStack(sceneRenderData.postEffects);
    renderer.setClearColor(sceneRenderData.backgroundColor);
}

void rendering::updateActivePerspectiveCamera(const PerspectiveCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, ViewportData& viewportData)
{
    float aspect = window.state->getFrameBufferAspect();

    viewportData.projectionMatrix = mat4::makePerspective(cameraData.fov, aspect, cameraData.near, cameraData.far);
    viewportData.viewMatrix = inverse(transform.getWorldMatrix());
    viewportData.cameraPos = transform.getWorldPosition();

    viewportData.pixelWidth = window.state->fbWidth;
    viewportData.pixelHeight = window.state->fbHeight;
}

void rendering::updateActiveOrthoCamera(const OrthoCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, ViewportData& viewportData)
{
    float aspect = window.state->getFrameBufferAspect();

    viewportData.projectionMatrix = mat4::makeOrtho(
        -cameraData.orthoSize * aspect,
        cameraData.orthoSize * aspect,
        -cameraData.orthoSize,
        cameraData.orthoSize,
        cameraData.near,
        cameraData.far
    );
    viewportData.viewMatrix = inverse(transform.getWorldMatrix());

    viewportData.pixelWidth = window.state->fbWidth;
    viewportData.pixelHeight = window.state->fbHeight;
}
