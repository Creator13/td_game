#include <spdlog/spdlog.h>

#include "engine.h"
#include "assets/Mesh.h"
#include "assets/Texture.h"
#include "core/Debug.h"
#include "core/EcsCore.h"
#include "core/EcsDebug.h"
#include "core/Input.h"
#include "core/Time.h"
#include "core/Transform.h"
#include "core/Window.h"
#include "core/ui/EcsUI.h"
#include "rendering/EcsRendering.h"
#include "rendering/Pipeline.h"

using namespace math;
using namespace core;
using namespace core::ecs;
using namespace core::assets;
using namespace core::gfx;

struct RotateData
{
    float angularVelocity;
};

struct DiscoLight
{
    float phaseOffset = 0.0f; // 0-1, randomize per-entity so lights desync
    float hueSpeed = 0.15f; // hue cycles per second
    float pulseSpeed = 1.2f; // intensity pulses per second
    float baseIntensity = 5.f;
    float pulseAmount = 0.8f; // 0 = steady, 1 = pulses all the way to baseIntensity*(1-pulseAmount)
};

struct VisualizeLight { };

WindowState engine::getInitialWindowState()
{
    return WindowState(1280, 720, "game", false);
}

void engine::setupGame(const flecs::world& world)
{
    const flecs::entity cam = world.entity("Debug camera")
        .set<PerspectiveCameraData>({60, .1, 1000})
        .add<ActiveCamera>();

    cam.get_ref<PerspectiveCameraData>();

    vec3 camPos = vec3(.2, -5, 3);
    transform::add(cam, camPos, quaternion::eulerAngles(-25, 0, 0));
    cam.set<FreeLookCameraControlData>({.targetSpeed = 5, .speedMultiplier = 1.75f});

    const AssetRef<Mesh> avocado = Mesh::loadFromFile("mesh/frischavacadoo.glb");
    const AssetRef<Mesh> sphere = Mesh::loadFromFile("mesh/primitive/uv_sphere.glb");
    const AssetRef<Mesh> cube = Mesh::loadFromFile("mesh/primitive/cube.glb");
    const AssetRef<Mesh> cubeSimpleUv = Mesh::loadFromFile("mesh/cubeSimpleUv.glb");
    const AssetRef<Mesh> groundPlane = Mesh::loadFromFile("mesh/primitive/plane.glb");

    AssetRef<Texture> containerDiffuse = Texture::loadFromFile("tex/container2.png", TextureFormat::RGBA8_SRGB, false, true);
    AssetRef<Texture> containerSpecular = Texture::loadFromFile("tex/container2_specular.png", TextureFormat::RGBA8_SRGB, false, true);
    AssetRef<Texture> uvCheckerTex = Texture::loadFromFile("tex/uv_checker.png", TextureFormat::RGBA8_SRGB, false, true);
    AssetRef<Texture> avacadooTex = Texture::loadFromFile("tex/Avocado_baseColor.png", TextureFormat::RGBA8_SRGB, false, true);

    constexpr PipelineDescriptor pDesc{
        .depthTest = true,
        .depthFunc = DepthFunction::Less,
        .blend = false,
        .backfaceCulling = BackfaceCulling::Back,
    };

    const AssetRef<Pipeline> tonemap = Pipeline::createFullscreenEffect("Tonemapping", "shaders/fullscreen/tonemap.fs.glsl");
    const auto tonemapMat = tonemap->newMaterialInstance("default");
    tonemapMat->setFloat("uExposure"_spid, 1);
    tonemapMat->setFloat("uGammaAdjust"_spid, 1);
    tonemapMat->setInt("uMode"_spid, 0);
    const AssetRef<Pipeline> fxaa = Pipeline::createFullscreenEffect("FXAA", "shaders/fullscreen/fxaa.fs.glsl");
    const auto fxaaMat = fxaa->newMaterialInstance("qw");

    SceneRenderData& sceneRenderData = world.get_mut<SceneRenderData>();
    sceneRenderData.postEffects = std::vector{tonemapMat, fxaaMat};
    sceneRenderData.backgroundColor = Color::fromSrgb(vec3(.07, .07, .1) * .5f);

    AssetRef<Pipeline> lit = Pipeline::create("lit", pDesc, "shaders/lit.v.glsl", "shaders/lit.f.glsl");
    auto baseMat = lit->newMaterialInstance("baseMat");
    baseMat->setFloat("shininess"_spid, 32);
    baseMat->setColor("diffuseColor"_spid, Color::coral);
    baseMat->setColor("specularColor"_spid, Color::white);

    // #### Lighting test scene
    AssetRef<Pipeline> unlitPipeline = Pipeline::create("unlit", pDesc, "shaders/basic.vert", "shaders/color.frag");
    AssetRef<Material> whiteUnlit = unlitPipeline->newMaterialInstance("whiteUnlit");
    whiteUnlit->setColor("color"_spid, Color::white);
    auto lightParent = world.entity("light parent")
        // .set<RotateData>({.angularVelocity = 25})
        ;
    transform::add(lightParent, vec3::zero);
    auto light = world.entity("light")
            .set<MeshRenderData>({.mesh = sphere, .material = whiteUnlit})
            .set<LightData>({.type = LightData::Type::Directional, .color = Color::white, .intensity = 1})
        // .set<DiscoLight>({})
        ;
    light.add<::debug::ecs::Gizmo>();
    transform::add(light, lightParent, vec3(1.2f, 1.0f, 2.0f), quaternion::lookRotation(vec3(1, 1, -.5), vec3::up), vec3(.1f));

    auto pointLight = world.entity("pointLight")
        .set<LightData>({.type = LightData::Type::Point, .color = Color::greenYellow, .intensity = 1, .range = 1});
    transform::add(pointLight, vec3(0, -1, .5f));

    auto containerMat = Material::duplicate(baseMat, "container");
    containerMat->setColor("diffuseColor"_spid, Color::white);
    containerMat->setTexture2D("diffuseTexture"_spid, containerDiffuse);
    containerMat->setTexture2D("specularTexture"_spid, containerSpecular);
    auto cube3 = world.entity("cube3")
        .set<MeshRenderData>({.mesh = cubeSimpleUv, .material = containerMat})
        .set<RotateData>({.angularVelocity = 15});
    constexpr float size = 1.2f;
    constexpr vec3 containerPos = vec3(-1, -2, 0.5 * size);
    transform::add(cube3, containerPos, quaternion::eulerAngles(0, 0, -14), vec3::one * size);

    auto spotlight = world.entity("spotlight")
        .set<LightData>({.type = LightData::Type::Spot, .color = Color::white, .intensity = 50, .cutoffDegrees = 25.f, .range = 15});
    constexpr vec3 spotlightPos = vec3(0, 0, 2.5f);
    transform::add(spotlight, spotlightPos, quaternion::lookRotation(containerPos - spotlightPos, vec3::up));

    AssetRef<Material> uvCheckerMat = Material::duplicate(baseMat, "mat");
    uvCheckerMat->setTexture2D("diffuseTexture"_spid, uvCheckerTex);
    uvCheckerMat->setColor("diffuseColor"_spid, Color::gray);
    auto floor = world.entity("Floor")
        .set<MeshRenderData>({.mesh = groundPlane, .material = uvCheckerMat});
    transform::add(floor, vec3(0, 0, 0));

    auto cube1 = world.entity("cube1")
        .set<MeshRenderData>({.mesh = cube, .material = baseMat});
    transform::add(cube1, vec3(-2, 3, 0.65), quaternion::eulerAngles(15, 0, 66));

    auto cube2 = world.entity("cube2")
        .set<MeshRenderData>({.mesh = cube, .material = baseMat});
    transform::add(cube2, vec3(2.3, 0, 0.5), quaternion::eulerAngles(0, 0, 37));

    auto sphereMat = Material::duplicate(baseMat, "sphere1");
    sphereMat->setColor("diffuseColor"_spid, Color::gray2);
    sphereMat->setFloat("shininess"_spid, 128);
    auto sphere1 = world.entity("sphere1")
        .set<MeshRenderData>({.mesh = sphere, .material = sphereMat});
    transform::add(sphere1, vec3(1, -1.5, .5));

    AssetRef<Material> avacadooMat = Material::duplicate(baseMat, "mat");
    avacadooMat->setTexture2D("diffuseTexture"_spid, avacadooTex);
    avacadooMat->setColor("diffuseColor"_spid, Color::white);
    auto avacadoo = world.entity("avacadoo")
        .set<MeshRenderData>({.mesh = avocado, .material = avacadooMat});
    transform::add(avacadoo, vec3(1, 1, .15), quaternion::eulerAngles(90, 0, 22), vec3::one * 10);

    // Gizmos
    PipelineDescriptor gizmosDesc = pDesc;
    gizmosDesc.depthTest = false;
    AssetRef<Pipeline> coloredGizmoShader = Pipeline::create("colored", gizmosDesc, "shaders/basic.vert", "shaders/color.frag");

    auto redGizmoMat = coloredGizmoShader->newMaterialInstance("red");
    redGizmoMat->setColor("color"_spid, Color::red);
    auto gizmoX = world.entity("gizmoX").set<MeshRenderData>({.mesh = cube, .material = redGizmoMat});
    transform::add(gizmoX, vec3::zero, quaternion::identity, vec3(4, .01, .01));

    auto greenGizmoMat = coloredGizmoShader->newMaterialInstance("green");
    greenGizmoMat->setColor("color"_spid, Color::green);
    auto gizmoY = world.entity("gizmoY").set<MeshRenderData>({.mesh = cube, .material = greenGizmoMat});
    transform::add(gizmoY, vec3::zero, quaternion::identity, vec3(.01, 4, .01));

    auto blueGizmoMat = coloredGizmoShader->newMaterialInstance("blue");
    blueGizmoMat->setColor("color"_spid, Color::blue);
    auto gizmoZ = world.entity("gizmoZ").set<MeshRenderData>({.mesh = cube, .material = blueGizmoMat});
    transform::add(gizmoZ, vec3::zero, quaternion::identity, vec3(.01, .01, 4));

    // #### NxN object scene
    // constexpr int count = 100;
    // int n = 0;
    // for (int i = 0; i < count; i++)
    // {
    //     for (int j = 0; j < count; j++, n++)
    //     {
    //         flecs::entity e;
    //         if (n % 3 == 0)
    //         {
    //             e = world.entity(fmt::format("avacadoo {}-{}", i, j).c_str())
    //                 .set<MeshRenderData>({.mesh = avocado, .material = avacadooMat});
    //         }
    //         else
    //         {
    //             e = world.entity(fmt::format("sphere {}-{}", i, j).c_str())
    //                 .set<MeshRenderData>({.mesh = sphere, .material = uvCheckerMat})
    //                 // .set<RotateData>({((i % 5) - 2) * 30.f})
    //                 ;
    //         }
    //         transform::add(e, vec3((i - count / 2) * 1.5f, (j - count / 2) * 1.5f, 0), quaternion::identity, vec3(1.f));
    //     }
    // }

    world.system<HierarchyTransform, const RotateData>("Rotating")
        .each([](flecs::iter& it, usize i, HierarchyTransform& transform, const RotateData& rotation)
        {
            if (math::abs(rotation.angularVelocity) < EPSILON) return;
            transform.rotate(it.entity(i), quaternion::eulerAngles(0, 0, rotation.angularVelocity * time::delta()));
        });

    world.system<LightData, DiscoLight>("Disco lights")
        .each([](flecs::entity e, LightData& light, DiscoLight& disco)
        {
            //LLM generated
            float t = time::sinceLoad();

            // Walk hue around the color wheel instead of wobbling R/G/B independently.
            float hue = math::fract(t * disco.hueSpeed + disco.phaseOffset);
            Color color = Color::fromSrgb(SrgbColor::fromHsv(hue, lerp(.5, 1, fract(t * 2.5)), 1));

            // Remap sine into [1-pulseAmount, 1] so intensity never goes negative
            // or snaps to zero, but still has a punchy pulse.
            float pulse = 0.5f + 0.5f * math::sin(t * disco.pulseSpeed + disco.phaseOffset * 6.2831853f);
            float intensity = disco.baseIntensity * (1.0f - disco.pulseAmount + disco.pulseAmount * pulse);

            light.color = color;
            light.intensity = intensity;

            if (e.has<MeshRenderData>())
            {
                const MeshRenderData& renderData = e.get<MeshRenderData>();
                renderData.material->setColor("color"_spid, color * intensity);
            }
        });

    world.system<HierarchyTransform, FreeLookCameraControlData, const GlobalInput>("Camera control")
        .with<ActiveCamera>()
        .each([](flecs::iter& it, usize i, HierarchyTransform& transform, FreeLookCameraControlData& camControl, const GlobalInput& input)
        {
            const flecs::entity e = it.entity(i);
            const float deltaTime = it.delta_time();

            const vec2 mouseDelta = input.state->mouseDelta;
            if (input.state->isMouseDown(MouseButton::Right))
            {
                camControl.yaw += -mouseDelta.x * camControl.sensitivity;
                camControl.pitch += -mouseDelta.y * camControl.sensitivity;

                transform.setGlobalOrientation(e, quaternion::eulerAngles(camControl.pitch, 0, camControl.yaw));
            }

            const vec3 forward = transform.getForward();
            const vec3 right = transform.getRight();
            const vec3 up = transform.getUp();

            vec3 dir;
            if (input.state->isKeyDown(Key::W)) dir += forward;
            if (input.state->isKeyDown(Key::S)) dir -= forward;

            if (input.state->isKeyDown(Key::D)) dir += right;
            if (input.state->isKeyDown(Key::A)) dir -= right;

            if (input.state->isKeyDown(Key::E)) dir += up;
            if (input.state->isKeyDown(Key::Q)) dir -= up;

            vec3 targetVelocity;
            if (dir == vec3::zero)
            {
                targetVelocity = vec3::zero;
            }
            else
            {
                targetVelocity = normalize(dir) * camControl.targetSpeed;
            }

            const float t = 1.0f - math::exp(-16.f * deltaTime);
            camControl.currentVelocity = lerp(camControl.currentVelocity, targetVelocity, t);

            vec3 move = camControl.currentVelocity * deltaTime;

            if (input.state->isKeyDown(Key::LeftShift))
            {
                move *= camControl.speedMultiplier;
            }
            if (input.state->isKeyDown(Key::LeftControl))
            {
                move /= camControl.speedMultiplier;
            }

            transform.translate(e, move);
        });

    // UI stuff concept
    // const flecs::entity ui = world.entity("UI root")
    //     .set<ui::UiRoot>({1920, 1080});

    // world.entity("Beautiful panel")
    //     .set<ui::Rect>({{500, 500}, {100, 50}, 0})
    //     .set<RotateData>({.angularVelocity = 15});

    world.system<ui::Rect, const RotateData>()
        .each([](ui::Rect& rect, const RotateData& rotation)
        {
            rect.rotation += rotation.angularVelocity * time::delta();
        });
}
