#include <spdlog/spdlog.h>

#include "engine.h"
#include "core/Application.h"
#include "core/EcsCore.h"
#include "core/Transform.h"
#include "core/Window.h"
#include "rendering/EcsRendering.h"

using namespace math;
using namespace core;
using namespace core::ecs;
using namespace assets;

struct RotateTag { };

WindowState engine::initial_window_state()
{
    return WindowState(1280, 720, "game", false);
}

void engine::register_flecs(const flecs::world& world)
{
    const flecs::entity cam = world.entity("Camera Test1")
        .set<PerspectiveCameraData>({75, .1, 100})
        .add<ActiveCamera>();

    transform::add(cam, vec3(.2, -5, 3) * 2, quaternion::eulerAngles(-25, 0, 0));
    cam.set<FreeLookCameraControlData>({ });

    const AssetId bunny = AssetDatabase::loadMeshFromFile("mesh/bunny.glb");
    const AssetId basicShader = AssetDatabase::loadShaderFromFiles("shaders/basic.vert", "shaders/basic.frag");
    AssetId uniformColorShader = AssetDatabase::loadShaderFromFiles("shaders/basic.vert", "shaders/color.frag");
    AssetId cubeMesh = idFromPath("@internal/mesh/cube");

    const flecs::entity bunny_prefab = world.prefab("Bunny")
        .set<MeshRenderer>({
            .meshId = bunny,
            .shaderId = basicShader
        })
        .set<MaterialData>({ });

    // const auto b1 = world.entity("bunny1")
    //     .is_a(bunny_prefab);
    // transform::add(b1, vec3::zero, quaternion::identity);
    //
    // const auto b2 = world.entity("bunny2")
    //     .is_a(bunny_prefab);
    // transform::add(b2, b1, vec3::right * 3, quaternion::identity);
    //
    // const auto b3 = world.entity("bunny3")
    //     .is_a(bunny_prefab);
    // transform::add(b3, b2, vec3::right * 5, quaternion::identity);
    //
    // b1.add<RotateTag>();

    world.system<HierarchyTransform>("Rotating")
        .with<RotateTag>()
        .each([](flecs::iter& it, size_t i, HierarchyTransform& transform)
        {
            transform.rotate(it.entity(i), quaternion::eulerAngles(0, 0, 30 * it.delta_time()));
        });

    world.system<HierarchyTransform, FreeLookCameraControlData, const GlobalInput>("Camera control")
        .with<ActiveCamera>()
        .each([](flecs::iter& it, size_t i, HierarchyTransform& transform, FreeLookCameraControlData& controlData, const GlobalInput& input)
        {
            const flecs::entity e = it.entity(i);
            const float deltaTime = it.delta_time();

            constexpr float speed = 5;

            const vec2 mouseDelta = input.state->mouseDelta;
            spdlog::debug("mouse delta: {}, {}", mouseDelta.x, mouseDelta.y);
            if (input.state->isMouseDown(MouseButton::Right))
            {
                controlData.yaw += mouseDelta.x * controlData.sensitivity;
                controlData.pitch += mouseDelta.y * controlData.sensitivity;

                transform.setGlobalOrientation(e, quaternion::eulerAngles(controlData.pitch, 0, controlData.yaw));
            }

            const vec3 forward = transform.getForward();
            const vec3 right = transform.getRight();
            const vec3 up = transform.getUp();

            debug::drawRay(vec3::zero, forward, graphics::Color::green);
            debug::drawRay(vec3::zero, right, graphics::Color::red);
            debug::drawRay(vec3::zero, up, graphics::Color::blue);

            vec3 dir;
            if (input.state->isKeyDown(Key::W))dir += forward;
            if (input.state->isKeyDown(Key::S)) dir -= forward;

            if (input.state->isKeyDown(Key::D)) dir += right;
            if (input.state->isKeyDown(Key::A)) dir -= right;

            if (input.state->isKeyDown(Key::E)) dir += up;
            if (input.state->isKeyDown(Key::Q)) dir -= up;

            transform.translate(e, dir * speed * deltaTime);
        });
}
