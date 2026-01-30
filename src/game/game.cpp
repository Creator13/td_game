#include <spdlog/spdlog.h>

#include "engine.h"
#include "assets/AssetDatabase.h"
#include "assets/Shader.h"
#include "assets/Texture.h"
#include "core/EcsCore.h"
#include "core/Input.h"
#include "core/Time.h"
#include "core/Transform.h"
#include "core/Window.h"
#include "rendering/EcsRendering.h"
#include "rendering/Mesh.h"

using namespace math;
using namespace core;
using namespace core::ecs;
using namespace core::assets;
using namespace graphics;

struct RotateData
{
    float angularVelocity;
};

WindowState engine::initial_window_state()
{
    return WindowState(1280, 720, "game", false);
}

Material mat;

void engine::register_flecs(const flecs::world& world)
{
    const flecs::entity cam = world.entity("Debug camera")
        .set<PerspectiveCameraData>({60, .1, 100})
        .add<ActiveCamera>();

    vec3 camPos = vec3(.2, -5, 3) * 2;
    transform::add(cam, camPos, quaternion::eulerAngles(-25, 0, 0));
    cam.set<FreeLookCameraControlData>({.targetSpeed = 5, .speedMultiplier = 1.75f});

    // const AssetRef<Mesh> avocado = Mesh::loadFromFile("mesh/frischavacadoo.glb");
    const AssetRef<Mesh> avocado = Mesh::loadFromFile("mesh/primitive/uv_sphere.glb");
    const AssetRef<Shader> texShader = Shader::fromFiles("shaders/textured.vert", "shaders/textured.frag");

    AssetRef<Texture> tex = Texture::loadFromFile("tex/uv_checker.png", TextureFormat::RGBA8_SRGB, false);

    mat.shader = texShader;
    mat.albedo = tex;
    mat.baseColor = Color::fromSrgb(SrgbColor::darkGreen);

    constexpr int count = 25;
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < count; j++)
        {
            auto e = world.entity(fmt::format("cube {}-{}", i, j).c_str())
                .set<MeshRenderData>({.mesh = avocado, .material = &mat})
                .set<RotateData>({((i % 5) - 2) * 30.f});
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
}
