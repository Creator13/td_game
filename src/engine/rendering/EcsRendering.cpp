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

        ecs.component<LightData::Type>("Light type")
            .constant("Directional", LightData::Type::Directional)
            .constant("Point", LightData::Type::Point)
            .constant("Spot", LightData::Type::Spot)
            .constant("Area", LightData::Type::Area);

        ecs.component<LightData>("Light")
            .member<LightData::Type>("Type")
            .member<core::Color>("Color")
            .member<float>("Intensity")
            .member<float>("Cutoff (degrees)")
            .member<float>("Range")
            .member<float>("Light score");

        ecs.component<ViewportData>().add(flecs::Singleton);
        ecs.component<ShadowData>().add(flecs::Singleton);
        ecs.component<SceneRenderData>().add(flecs::Singleton);
        ecs.component<WindowSingleton>().add(flecs::Singleton);
        ecs.component<RendererSingleton>().add(flecs::Singleton);
    }

    int sortLightsComparison(flecs::entity_t e1, const LightData* l1, flecs::entity_t e2, const LightData* l2)
    {
        (void) e1;
        (void) e2;

        if (l1->type != l2->type)
        {
            return static_cast<int>(l1->type) - static_cast<int>(l2->type);
        }

        return (l1->score < l2->score) - (l1->score > l2->score);
    }

    constexpr float MAX_LIGHT_DISTANCE_SQR = 50;

    float scorePointLight(const LightData& light, const HierarchyTransform& transform, const ViewportData& viewportData)
    {
        ZoneScoped

        // Cull lights outside view frustum
        if (!isSphereInFrustum({.center = transform.getWorldPosition(), .radius = light.range}, viewportData.frustum))
        {
            return -1;
        }

        const vec3 toLight = transform.getWorldPosition() - viewportData.cameraPos;
        const float sqrDist = sqrDistance(transform.getWorldPosition(), viewportData.cameraPos);

        // Cull when out of light draw distance
        if (sqrDist > MAX_LIGHT_DISTANCE_SQR * MAX_LIGHT_DISTANCE_SQR)
        {
            return -1;
        }

        // Scoring
        constexpr float minSqrDist = 0.1f;
        const float effectiveSqrDist = max(sqrDist, minSqrDist);

        const float dist = math::sqrt(sqrDist);
        const vec3 dirToLight = (dist > 0.0001f) ? (toLight / dist) : viewportData.viewDir;
        const float viewDot = dot(viewportData.viewDir, dirToLight);
        const float viewFactor = clamp(0.5f + 0.5f * viewDot, 0.05f, 1.0f);

        const float brightness = std::max({light.color.r, light.color.g, light.color.b});
        return (brightness * light.intensity * viewFactor) / (effectiveSqrDist * effectiveSqrDist);
    }

    float scoreSpotlight(const LightData& light, const HierarchyTransform& transform, const ViewportData& viewportData)
    {
        ZoneScoped

        constexpr float cos45 = 0.70710678f;
        const vec3 lightDir = transform.getForward();
        const vec3 lightPos = transform.getWorldPosition();

        const float outerCutoffCos = math::cos(light.cutoffDegrees * .5f * DEG2RAD);

        // Frustum culling
        sphere boundingSphere;
        if (outerCutoffCos >= cos45)
        {
            const float t = light.range / (2.0f * outerCutoffCos * outerCutoffCos);
            boundingSphere = {.center = lightPos + lightDir * t, .radius = t};
        }
        else
        {
            const float sinHalfAngle = math::sqrt(max(1.0f - outerCutoffCos * outerCutoffCos, 0.0f));
            const float baseRadius = light.range * (sinHalfAngle / outerCutoffCos);
            boundingSphere = {.center = lightPos + lightDir * light.range, .radius = baseRadius};
        }

        if (!isSphereInFrustum(boundingSphere, viewportData.frustum))
        {
            return -1;
        }

        const vec3 toLight = lightPos - viewportData.cameraPos;
        const float sqrDist = sqrDistance(lightPos, viewportData.cameraPos);

        // Cull when out of light draw distance
        if (sqrDist > MAX_LIGHT_DISTANCE_SQR * MAX_LIGHT_DISTANCE_SQR)
        {
            return -1;
        }

        // Scoring
        constexpr float minSqrDist = 0.1f;
        const float effectiveSqrDist = max(sqrDist, minSqrDist);

        const float dist = math::sqrt(sqrDist);
        const vec3 dirToLight = (dist > 0.0001f) ? (toLight / dist) : viewportData.viewDir;
        const float viewDot = dot(viewportData.viewDir, dirToLight);
        const float viewFactor = clamp(0.5f + 0.5f * viewDot, 0.05f, 1.0f);

        const float brightness = std::max({light.color.r, light.color.g, light.color.b});
        return (brightness * light.intensity * viewFactor) / (effectiveSqrDist * effectiveSqrDist);
    }

    void scoreLight(LightData& light, const HierarchyTransform& transform, const ViewportData& viewportData)
    {
        switch (light.type)
        {
            case LightData::Type::Directional:
                light.score = light.intensity;
                break;
            case LightData::Type::Point:
                light.score = scorePointLight(light, transform, viewportData);
                break;
            case LightData::Type::Spot:
                light.score = scoreSpotlight(light, transform, viewportData);
                break;
            default:
                light.score = light.intensity;
                break;
        }
    }

    void collectLights(flecs::iter& iter)
    {
        ZoneScopedN("Light collection")

        const auto& renderer = iter.world().get<const RendererSingleton>();

        const LightData* mainDirLight = nullptr;
        const HierarchyTransform* mainLightTransform = nullptr;
        int numPointLights = 0;
        int numDirectionalLights = 0;
        int numSpotlights = 0;

        while (iter.next())
        {
            auto f_lightData = iter.field<const LightData>(0);
            auto f_transform = iter.field<const HierarchyTransform>(1);

            for (const auto i : iter)
            {
                const HierarchyTransform& transform = f_transform[i];
                const LightData& ecsLight = f_lightData[i];

                // Skip culled lights (determined by scoring system;
                // geometrically culled lights get a score of -1 and all lights are removed if their intensity is 0)
                if (ecsLight.score <= 0) continue;

                switch (ecsLight.type)
                {
                    case LightData::Type::Directional:
                    {
                        if (numDirectionalLights >= LightingDataBlock::MAX_LIGHTS) break;

                        LightingDataBlock::DirectionalLight dirLight{
                            .direction = transform.getForward(),
                            .color = ecsLight.color.rgbVec3(),
                            .intensity = ecsLight.intensity
                        };
                        if (!mainDirLight)
                        {
                            mainDirLight = &ecsLight;
                            mainLightTransform = &transform;
                        }
                        renderer.ptr->submitDirectionalLight(dirLight);
                        numDirectionalLights++;
                        break;
                    }
                    case LightData::Type::Point:
                    {
                        if (numPointLights >= LightingDataBlock::MAX_LIGHTS) break;

                        LightingDataBlock::PointLight pointLight{
                            .position = transform.getWorldPosition(),
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
                        if (numSpotlights >= LightingDataBlock::MAX_LIGHTS) break;

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
                        ENGINE_ASSERT(false, "Unsupported light type: {}", static_cast<std::underlying_type_t<LightData::Type>>(ecsLight.type));
                        break;
                }
            }
        }

        // Set shadow casting properties
        ShadowData& shadowData = iter.world().get_mut<ShadowData>();
        shadowData.renderShadows = mainDirLight && mainLightTransform;
        if (shadowData.renderShadows)
        {
            shadowData.mainLightPos= mainLightTransform->getWorldPosition();
            shadowData.mainLightDir = mainLightTransform->getForward();
            // It is a minor optimization to create the view matrix as a TRS, avoids one multiplication and creates the matrix in-place
            shadowData.viewMatrix = inverse(mat4::makeTRS(-shadowData.mainLightDir * 10, rot3x3::lookRotation(shadowData.mainLightDir, vec3::up), vec3::one));
            shadowData.projectionMatrix = mat4::makeOrtho(-10.f, 10.f, -10.f, 10.f, .1f, 30.f);
            shadowData.viewProjectionMatrix = shadowData.projectionMatrix * constants::COORDINATE_BASIS * shadowData.viewMatrix;
            shadowData.mainLightFrustum = frustum::fromViewProjectionMatrix(shadowData.viewProjectionMatrix);
        }
        renderer.ptr->setShadowData(shadowData);
    }

    // TODO find a solution for this that I love more (CurrentActiveCamera with an entity target?)
    flecs::entity currentActiveCameraEntity;
}

rendering::rendering(flecs::world& ecs)
{
    ecs.module<rendering>("Rendering");

    registerComponents(ecs);

    ecs.set<ViewportData>({ });
    ecs.set<ShadowData>({ });
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

    const auto perspSystem = ecs.system<const PerspectiveCameraData, const HierarchyTransform, const WindowSingleton, ViewportData>("Perspective camera update system")
        .kind(flecs::PreStore)
        .with<ActiveCamera>()
        .each(updateActivePerspectiveCamera);

    const auto orthoSystem = ecs.system<const OrthoCameraData, const HierarchyTransform, const WindowSingleton, ViewportData>("Ortho camera update system")
        .kind(flecs::PreStore)
        .with<ActiveCamera>()
        .each(updateActiveOrthoCamera);

    const auto viewportSyncSystem = ecs.system<const RendererSingleton, const ViewportData>()
        .each(syncRendererToActiveCamera)
        .depends_on(perspSystem)
        .depends_on(orthoSystem);

    ecs.system<const RendererSingleton, const SceneRenderData>("Scene data synchronization system")
        .kind(flecs::PreStore)
        .each(syncSceneData);

    const auto lightScoringSystem = ecs.system<LightData, const HierarchyTransform, const ViewportData>("Light scoring system")
        // TODO profile
        .each(scoreLight)
        .depends_on(viewportSyncSystem);

    const auto lightCollectionSystem = ecs.system<const LightData, const HierarchyTransform>("Light collection system")
        .kind(flecs::OnStore)
        .order_by(sortLightsComparison)
        .run(collectLights)
        .depends_on(lightScoringSystem);

    const auto boundsCalculationSystem = ecs.system<const HierarchyTransform, BoxBoundsData>()
        .kind(flecs::OnStore)
        .multi_threaded()
        .each([](const HierarchyTransform& transform, BoxBoundsData& bounds)
        {
            if (bounds.cachedTransformVersion != transform.version)
            {
                const mat4& worldMat = transform.getWorldMatrix();

                AABB& worldBounds = bounds.cachedWorldBounds;
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
            }
        });

    const auto cullingSystem = ecs.system<const BoxBoundsData, MeshRenderData>("Culling system")
        .kind(flecs::OnStore)
        .multi_threaded()
        .run([](flecs::iter& it)
        {
            ZoneScopedN("Culling system");

            auto& viewport = it.world().get<const ViewportData>();
            auto& shadowData = it.world().get<const ShadowData>();

            while (it.next())
            {
                auto f_bounds = it.field<const BoxBoundsData>(0);
                auto f_renderData = it.field<MeshRenderData>(1);

                for (const auto i : it)
                {
                    const BoxBoundsData& bounds = f_bounds[i];
                    MeshRenderData& renderData = f_renderData[i];

                    renderData.cullReason = CullReason::None;

                    // Cull entity if it falls outside the view frustum
                    if (!isAABBInFrustum(bounds.cachedWorldBounds, viewport.frustum))
                    {
                        renderData.cullReason = CullReason::Frustum;
                    }

                    renderData.cullShadowCasting = !isAABBInFrustum(bounds.cachedWorldBounds, shadowData.mainLightFrustum);
                }
            }
        })
        .depends_on(lightCollectionSystem)
        .depends_on(boundsCalculationSystem);

    ecs.system<const HierarchyTransform, const MeshRenderData>("Scene geometry collection")
        // .multi_threaded() // TODO make multithreaded (but obv can't while renderer doesn't have a thread-safe render list)
        .kind(flecs::OnStore)
        .run([](flecs::iter& iter)
        {
            ZoneScopedN("Scene geometry collection system");

            const auto& renderer = iter.world().get<const RendererSingleton>();
            const auto& shadowData = iter.world().get<const ShadowData>();

            while (iter.next())
            {
                auto f_transform = iter.field<const HierarchyTransform>(0);
                auto f_renderData = iter.field<const MeshRenderData>(1);

                for (const auto i : iter)
                {
                    const auto& renderData = f_renderData[i];

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

                    if (shadowData.renderShadows && !renderData.cullShadowCasting && renderData.castShadow)
                    {
                        // TODO implement shadow casting of object as override of pipeline setting
                        renderer.ptr->submitShadowCommand(renderData.mesh, f_transform[i].getWorldMatrix());
                    }
                }
            }
        })
        .depends_on(cullingSystem);
}

void rendering::syncRendererToActiveCamera(const RendererSingleton& rPtr, const ViewportData& viewportData)
{
    Renderer& renderer = *rPtr.ptr;
    renderer.setViewportData(viewportData);
}

void rendering::syncSceneData(const RendererSingleton& rPtr, const SceneRenderData& sceneRenderData)
{
    Renderer& renderer = *rPtr.ptr;
    renderer.setPostEffectStack(sceneRenderData.postEffects);
    renderer.setClearColor(sceneRenderData.backgroundColor);
}

void rendering::updateActivePerspectiveCamera(const PerspectiveCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, ViewportData& viewportData)
{
    const float aspect = window.state->getFrameBufferAspect();

    viewportData.projectionMatrix = mat4::makePerspective(cameraData.fov, aspect, cameraData.near, cameraData.far);
    viewportData.viewMatrix = inverse(transform.getWorldMatrix());
    viewportData.viewProjectionMatrix = viewportData.projectionMatrix * constants::COORDINATE_BASIS * viewportData.viewMatrix;
    viewportData.cameraPos = transform.getWorldPosition();
    viewportData.frustum = frustum::fromViewProjectionMatrix(viewportData.viewProjectionMatrix);

    viewportData.pixelWidth = window.state->fbWidth;
    viewportData.pixelHeight = window.state->fbHeight;
}

void rendering::updateActiveOrthoCamera(const OrthoCameraData& cameraData, const HierarchyTransform& transform, const WindowSingleton& window, ViewportData& viewportData)
{
    const float aspect = window.state->getFrameBufferAspect();

    viewportData.projectionMatrix = mat4::makeOrtho(
        -cameraData.orthoSize * aspect,
        cameraData.orthoSize * aspect,
        -cameraData.orthoSize,
        cameraData.orthoSize,
        cameraData.near,
        cameraData.far
    );
    viewportData.viewMatrix = inverse(transform.getWorldMatrix());
    viewportData.viewProjectionMatrix = viewportData.projectionMatrix * constants::COORDINATE_BASIS * viewportData.viewMatrix;
    viewportData.cameraPos = transform.getWorldPosition();
    viewportData.frustum = frustum::fromViewProjectionMatrix(viewportData.viewProjectionMatrix);

    viewportData.pixelWidth = window.state->fbWidth;
    viewportData.pixelHeight = window.state->fbHeight;
}
