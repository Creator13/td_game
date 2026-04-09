#include <spdlog/spdlog.h>

#include "engine.h"
#include "assets/AssetDatabase.h"
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
        .set<PerspectiveCameraData>({60, .1, 100})
        .add<ActiveCamera>();

    cam.get_ref<PerspectiveCameraData>();

    vec3 camPos = vec3(.2, -5, 3) * 2;
    transform::add(cam, camPos, quaternion::eulerAngles(-25, 0, 0));
    cam.set<FreeLookCameraControlData>({.targetSpeed = 5, .speedMultiplier = 1.75f});

    const AssetRef<Mesh> avocado = Mesh::loadFromFile("mesh/frischavacadoo.glb");
    const AssetRef<Mesh> sphere = Mesh::loadFromFile("mesh/primitive/uv_sphere.glb");

    AssetRef<Texture> uvCheckerTex = Texture::loadFromFile("tex/uv_checker.png", TextureFormat::RGBA8_SRGB, false);
    AssetRef<Texture> avacadoo = Texture::loadFromFile("tex/Avocado_baseColor.png", TextureFormat::RGBA8_SRGB, false);

    PipelineDescriptor pDesc;
    pDesc.blend = false;
    pDesc.depthTest = true;
    pDesc.backfaceCulling = BackfaceCulling::Back;

    AssetRef<Pipeline> pipeline = Pipeline::create("textured", pDesc, "shaders/textured.vert", "shaders/textured.frag");

    AssetRef<Material> uvCheckerMat = pipeline->newMaterialInstance("mat");
    uvCheckerMat->setTexture2D("_mainTex"_spid, uvCheckerTex);
    uvCheckerMat->setColor("color"_spid, Color::fromSrgb(SrgbColor::green));

    AssetRef<Material> avacadooMat = pipeline->newMaterialInstance("mat");
    avacadooMat->setTexture2D("_mainTex"_spid, avacadoo);

    // spdlog::debug("uvcheckerpath: {}, avacadoo path: {}", AssetDatabase::getAssetInfo(uvCheckerMat).path, AssetDatabase::getAssetInfo(avacadooMat).path);

    constexpr int count = 50;
    int n = 0;
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < count; j++, n++)
        {
            flecs::entity e;
            if (n % 3 == 0)
            {
                e = world.entity(fmt::format("avacadoo {}-{}", i, j).c_str())
                    .set<MeshRenderData>({.mesh = avocado, .material = avacadooMat});
            }
            else
            {
                e = world.entity(fmt::format("sphere {}-{}", i, j).c_str())
                    .set<MeshRenderData>({.mesh = sphere, .material = uvCheckerMat})
                    .set<RotateData>({((i % 5) - 2) * 30.f});
            }
            transform::add(e, vec3((i - count / 2) * 1.5f, (j - count / 2) * 1.5f, 0), quaternion::identity, vec3(1.f));
        }
    }

    world.system<HierarchyTransform, const RotateData>("Rotating")
        .each([](flecs::iter& it, usize i, HierarchyTransform& transform, const RotateData& rotation)
        {
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

    // const flecs::entity text = world.entity("Text")
    //     .emplace<ui::Text>("Yes")
    //     .set<ui::Rect>({{10, 10}, {50, 50}});

    world.entity("Beautiful panel")
        .set<ui::Rect>({{50, 100}, {100, 50}});
}
