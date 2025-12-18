#include "core/EcsCore.h"

#include <flecs.h>

#include "Input.h"
#include "core/transform.h"

using namespace math;
using namespace core::ecs;

namespace
{
    void registerComponents(flecs::world& ecs)
    {
        ecs.component<vec3>()
            .member<float>("x")
            .member<float>("y")
            .member<float>("z");

        ecs.component<quaternion>()
            .member<float>("x")
            .member<float>("y")
            .member<float>("z")
            .member<float>("w");

        auto worldTransform = ecs.component<WorldTransformData>();

        ecs.component<TransformData>("Transform")
            .member<vec3>("Position")
            .member<quaternion>("Rotation")
            .member<vec3>("Scale")
            .add(flecs::With, worldTransform);

        ecs.component<GlobalInput>().add(flecs::Singleton);
    }

    namespace _systems
    {

    }
}

engine_core::engine_core(flecs::world& ecs)
{
    ecs.module<engine_core>("Core module");
    registerComponents(ecs);

    ecs.system<TransformData, const WorldTransformData*, WorldTransformData>("World transform updating")
        .term_at(1).parent().cascade()
        .kind(flecs::OnValidate)
        .run([](flecs::iter& it)
        {
            while (it.next())
            {
                flecs::field<TransformData> f_local = it.field<TransformData>(0);
                flecs::untyped_field f_parent = it.field(1);
                flecs::field<WorldTransformData> f_world = it.field<WorldTransformData>(2);

                for (auto i : it)
                {
                    TransformData& local = f_local[i];
                    const WorldTransformData* parent = static_cast<const WorldTransformData*>(f_parent[i]);
                    WorldTransformData& world = f_world[i];

                    world.propagateChange = false;

                    mat4 localTRS = local.ensureTRS();


                    if (parent != nullptr && parent->propagateChange)
                    {
                        world.worldTransformMatrix = parent->worldTransformMatrix * localTRS;
                        world.propagateChange = true;
                    }
                    else if (local.isDirty())
                    {
                        world.worldTransformMatrix = localTRS;
                        world.propagateChange = true;
                    }

                    // We updated the world transform if needed, so the entity transform is clean now
                    local.markClean();
                }
            }
        });
    // [](TransformData& local, const WorldTransformData* parent, WorldTransformData& world)
    // {
    // world.propagateChange = false;
    //
    // mat4 localTRS = local.ensureTRS();
    //
    // if (parent != nullptr && parent->propagateChange)
    // {
    //     world.worldTransformMatrix = parent->worldTransformMatrix * localTRS;
    //     world.propagateChange = true;
    // }
    // else if (local.isDirty())
    // {
    //     world.worldTransformMatrix = localTRS;
    //     world.propagateChange = true;
    // }
    //
    // // We updated the world transform if needed, so the entity transform is clean now
    // local.markClean();
    // });
}
