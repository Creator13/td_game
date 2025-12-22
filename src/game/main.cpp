#include <spdlog/spdlog.h>

#include "core/Application.h"
#include "core/Transform.h"
#include "rendering/EcsRendering.h"

using namespace core::ecs;

struct RotateTag { };

int main(int argc, char* argv[])
{
    auto game = core::Application(argc, argv, "res", core::WindowState(1280, 720, "td_game", false));

    flecs::world& world = game.getEcsWorld();
    core::TransformSystem& t = game.getTransformSys();

    flecs::entity cam = world.entity("Camera Test1")
        .set<PerspectiveCameraData>({75, .1, 100})
        .add<ActiveCamera>();

    t.addTransform(cam,
        math::vec3(.2, -5, 3) * 2,
        math::quaternion::eulerAngles(-25, 0, 0));

    assets::AssetDatabase& db = game.assets();
    assets::AssetId bunny = db.loadMeshFromFile("mesh/bunny.glb");
    assets::AssetId basicShader = db.loadShaderFromFiles("shaders/basic.vert", "shaders/basic.frag");
    assets::AssetId uniformColorShader = db.loadShaderFromFiles("shaders/basic.vert", "shaders/color.frag");
    assets::AssetId cubeMesh = assets::idFromPath("@internal/mesh/cube");

    flecs::entity bunny_prefab = world.prefab("Bunny")
        .set<MeshRenderer>({
            .meshId = bunny,
            .shaderId = basicShader
        })
        .set<MaterialData>({ });

    auto b1 = world.entity("bunny1")
        .is_a(bunny_prefab);
    auto t1 = t.addTransform(b1, math::vec3::zero, math::quaternion::identity);

    auto b2 = world.entity("bunny2")
        .is_a(bunny_prefab);
    auto t2 = t.addTransform(b2, t1, math::vec3::right * 3, math::quaternion::identity);

    auto b3 = world.entity("bunny3")
        .is_a(bunny_prefab);
    auto t3 = t.addTransform(b3, t2, math::vec3::right * 3, math::quaternion::identity);

    b1.add<RotateTag>();

    world.system<const core::TransformHandle>("Rotating")
        .with<RotateTag>()
        .each([&t](flecs::iter& it, size_t, const core::TransformHandle& handle)
        {
            t.rotate(handle, math::quaternion::eulerAngles(0, 0, 30 * it.delta_time()));
        });

    world.system<const core::TransformHandle, const GlobalInput>("Camera control")
        .with<ActiveCamera>()
        .each([&t](flecs::iter& it, size_t, const core::TransformHandle& handle, const GlobalInput& input)
        {
            constexpr float speed = 5;
            if (input.state->isKeyDown(core::Key::A))
            {
                t.translate(handle, math::vec3(speed, 0, 0) * it.delta_time());
            }
            else if (input.state->isKeyDown(core::Key::D))
            {
                t.translate(handle, math::vec3(-speed, 0, 0) * it.delta_time());
            }

            if (input.state->isMouseDown(core::MouseButton::Right))
            {
                t.translate(handle, math::vec3(0, speed, 0) * it.delta_time());
            }

            if (input.state->isMousePressed(core::MouseButton::Left))
            {
                spdlog::info("left button pressed");
            }
            if (input.state->isMouseReleased(core::MouseButton::Left))
            {
                spdlog::info("left button released");
            }
        });

    game.run();
}
