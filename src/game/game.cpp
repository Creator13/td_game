#include <spdlog/spdlog.h>

#include "engine.h"
#include "core/Application.h"
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
    flecs::entity cam = world.entity("Camera Test1")
        .set<PerspectiveCameraData>({75, .1, 100})
        .add<ActiveCamera>();

    transform::add(cam, vec3(.2, -5, 3) * 2, quaternion::eulerAngles(-25, 0, 0));

    AssetId bunny = AssetDatabase::loadMeshFromFile("mesh/bunny.glb");
    AssetId basicShader = AssetDatabase::loadShaderFromFiles("shaders/basic.vert", "shaders/basic.frag");
    AssetId uniformColorShader = AssetDatabase::loadShaderFromFiles("shaders/basic.vert", "shaders/color.frag");
    AssetId cubeMesh = idFromPath("@internal/mesh/cube");

    flecs::entity bunny_prefab = world.prefab("Bunny")
        .set<MeshRenderer>({
            .meshId = bunny,
            .shaderId = basicShader
        })
        .set<MaterialData>({ });

    auto b1 = world.entity("bunny1")
        .is_a(bunny_prefab);
    transform::add(b1, vec3::zero, quaternion::identity);

    auto b2 = world.entity("bunny2")
        .is_a(bunny_prefab);
    transform::add(b2, b1, vec3::right * 3, quaternion::identity);

    auto b3 = world.entity("bunny3")
        .is_a(bunny_prefab);
    transform::add(b3, b2, vec3::right * 5, quaternion::identity);

    b1.add<RotateTag>();

    world.system<HierarchyTransform>("Rotating")
        .with<RotateTag>()
        .each([](flecs::iter& it, size_t i, HierarchyTransform& transform)
        {
            transform.rotate(it.entity(i), quaternion::eulerAngles(0, 0, 30 * it.delta_time()));
        });

    world.system<HierarchyTransform, const GlobalInput>("Camera control")
        .with<ActiveCamera>()
        .each([](flecs::iter& it, size_t i, HierarchyTransform& transform, const GlobalInput& input)
        {
            flecs::entity e = it.entity(i);

            constexpr float speed = 5;
            if (input.state->isKeyDown(Key::A))
            {
                transform.translate(e, vec3(speed, 0, 0) * it.delta_time());
            }
            else if (input.state->isKeyDown(Key::D))
            {
                transform.translate(e, vec3(-speed, 0, 0) * it.delta_time());
            }

            if (input.state->isMouseDown(MouseButton::Right))
            {
                transform.translate(e, vec3(0, speed, 0) * it.delta_time());
            }

            if (input.state->isMousePressed(MouseButton::Left))
            {
                spdlog::info("left button pressed");
            }
            if (input.state->isMouseReleased(MouseButton::Left))
            {
                spdlog::info("left button released");
            }
        });
}
