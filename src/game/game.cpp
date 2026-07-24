#include <spdlog/spdlog.h>

#include "engine.h"
#include "assets/Mesh.h"
#include "assets/Texture.h"
#include "core/EcsCore.h"
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
    const AssetRef<Mesh> groundPlane = Mesh::loadFromFile("mesh/primitive/plane.glb");

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
    baseMat->setFloat("specularStrength"_spid, 0.5);
    baseMat->setColor("baseColor"_spid, Color::coral);

    // #### Lighting test scene
    AssetRef<Pipeline> unlitPipeline = Pipeline::create("unlit", pDesc, "shaders/basic.vert", "shaders/color.frag");
    AssetRef<Material> whiteUnlit = unlitPipeline->newMaterialInstance("whiteUnlit");
    whiteUnlit->setColor("color"_spid, Color::white);
    auto lightParent = world.entity("light parent").set<RotateData>({.angularVelocity = 25});
    transform::add(lightParent, vec3::zero);
    auto light = world.entity("light")
        .set<MeshRenderData>({.mesh = sphere, .material = whiteUnlit})
        .set<LightData>({.color = Color::white, .strength = 1});
    transform::add(light, lightParent, vec3(1.2f, 1.0f, 2.0f), quaternion::identity, vec3(.1f));

    AssetRef<Material> uvCheckerMat = Material::duplicate(baseMat, "mat");
    uvCheckerMat->setTexture2D("baseTexture"_spid, uvCheckerTex);
    uvCheckerMat->setColor("baseColor"_spid, Color::gray);
    auto floor = world.entity("Floor")
        .set<MeshRenderData>({.mesh = groundPlane, .material = uvCheckerMat});
    transform::add(floor, vec3(0, 0, 0));

    auto cube1Mat = Material::duplicate(baseMat, "cube1");
    cube1Mat->setColor("baseColor"_spid, Color::blanchedAlmond);
    auto cube1 = world.entity("cube1")
        .set<MeshRenderData>({.mesh = cube, .material = cube1Mat});
    transform::add(cube1, vec3(-2, 3, 0.65), quaternion::eulerAngles(15, 0, 66));

    auto cube2 = world.entity("cube2")
        .set<MeshRenderData>({.mesh = cube, .material = baseMat});
    transform::add(cube2, vec3(2.3, 0, 0.5), quaternion::eulerAngles(0, 0, 37));

    auto cube3 = world.entity("cube3")
        .set<MeshRenderData>({.mesh = cube, .material = baseMat})
        .set<RotateData>({.angularVelocity = 15});
    transform::add(cube3, vec3(-1, -2, 0.5), quaternion::eulerAngles(0, 0, -14), vec3::one * 1.5f);

    auto sphereMat = Material::duplicate(baseMat, "sphere1");
    sphereMat->setColor("baseColor"_spid, Color::gray2);
    sphereMat->setFloat("specularStrength"_spid, 1);
    auto sphere1 = world.entity("sphere1")
        .set<MeshRenderData>({.mesh = sphere, .material = sphereMat});
    transform::add(sphere1, vec3(1, -1.5, .5));

    AssetRef<Material> avacadooMat = Material::duplicate(baseMat, "mat");
    avacadooMat->setTexture2D("baseTexture"_spid, avacadooTex);
    avacadooMat->setColor("baseColor"_spid, Color::white);
    auto avacadoo = world.entity("avacadoo")
        .set<MeshRenderData>({.mesh = avocado, .material = avacadooMat});
    transform::add(avacadoo, vec3(1, 1, .15), quaternion::eulerAngles(90, 0, 22), vec3::one * 10);

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
