#include <GLFW/glfw3.h>

#include "../../build/release/vcpkg_installed/x64-windows-static-md-release/include/spdlog/spdlog.h"
#include "core/Application.h"
#include "core/transform.h"
#include "rendering/EcsRendering.h"

using namespace core::ecs;

struct RotateTag { };

int main(int argc, char* argv[])
{
    auto game = core::Application(argc, argv, "res", core::WindowState(1280, 720, "td_game", false));

    flecs::world& world = game.getEcsWorld();

    world.entity("Camera Test1")
        .set<TransformData>({
            math::vec3(.2, -5, 3) * 2,
            math::quaternion::eulerAngles(-25, 0, 0),
            math::vec3::one
        })
        .set<PerspectiveCameraData>({75, .1, 100})
        .add<ActiveCamera>();

    assets::AssetDatabase& db = game.assets();
    assets::AssetId bunny = db.loadMeshFromFile("mesh/bunny.glb");
    assets::AssetId basicShader = db.loadShaderFromFiles("shaders/basic.vert", "shaders/basic.frag");
    assets::AssetId uniformColorShader = db.loadShaderFromFiles("shaders/basic.vert", "shaders/color.frag");
    assets::AssetId cubeMesh = assets::AssetDatabase::idFromPath("@internal/mesh/cube");

    auto bunny_prefab = world.prefab("Bunny")
        .set<TransformData>({
            math::vec3::right * 3,
            math::quaternion::identity,
            math::vec3::one
        })
        .set<MeshRenderer>({
            .meshId = bunny,
            .shaderId = basicShader
        })
        .set<MaterialData>({ });

    auto b1 = world.entity("bunny1")
        .set<TransformData>({math::vec3::zero, math::quaternion::identity, math::vec3::one})
        .is_a(bunny_prefab);
    auto b2 = world.entity("bunny2")
        .child_of(b1)
        .is_a(bunny_prefab);
    auto b3 = world.entity("bunny3")
        .child_of(b2)
        .is_a(bunny_prefab);

    b1.add<RotateTag>();

    world.system<TransformData>("Rotating")
        .with<RotateTag>()
        .each([](flecs::iter& it, size_t idx, TransformData& t)
        {
            t.rotate(math::quaternion::eulerAngles(0, 0, 30 * it.delta_time()));
        });

    world.system<TransformData, const GlobalInput>("Camera control")
        .with<ActiveCamera>()
        .each([&game](flecs::iter& it, size_t, TransformData& transform, const GlobalInput& input)
        {
            constexpr float speed = 5;
            if (input.input->isKeyDown(core::Key::A))
            {
                transform.translate(math::vec3(speed, 0, 0) * it.delta_time());
            }
            else if (input.input->isKeyDown(core::Key::D))
            {
                transform.translate(math::vec3(-speed, 0, 0) * it.delta_time());
            }

            int qPressed = glfwGetKey(const_cast<GLFWwindow*>(game.getWindowPtr()), GLFW_KEY_Q);
            if (qPressed == GLFW_PRESS || qPressed == GLFW_RELEASE)
            {
                if (input.input->isKeyPressed(core::Key::Q))
                {
                    spdlog::info("Q pressed");
                }
                if (input.input->isKeyReleased(core::Key::Q))
                {
                    spdlog::info("Q released");
                }
            }
        });

    // world.entity("x axis")
    //     .set<TransformData>({
    //         math::vec3::zero,
    //         math::quaternion::identity,
    //         math::vec3(10, .05, .05)
    //     })
    //     .set<MeshRenderer>({cubeMesh, uniformColorShader})
    //     .set<MaterialData>({graphics::Color::red});
    //
    // world.entity("y axis")
    //     .set<TransformData>({
    //         math::vec3::zero,
    //         math::quaternion::identity,
    //         math::vec3(.05, 10, .05)
    //     })
    //     .set<MeshRenderer>({cubeMesh, uniformColorShader})
    //     .set<MaterialData>({graphics::Color::green});
    //
    // world.entity("z axis")
    //     .set<TransformData>(
    //         {
    //             math::vec3::zero,
    //             math::quaternion::identity,
    //             math::vec3(.05, .05, 10)
    //         })
    //     .set<MeshRenderer>({cubeMesh, uniformColorShader})
    //     .set<MaterialData>({graphics::Color::blue});

    game.run();
}
