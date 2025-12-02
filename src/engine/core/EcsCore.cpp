#include "core/EcsCore.h"

#include <flecs.h>

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
}
}

engine_core::engine_core(flecs::world& ecs)
{
    ecs.module<engine_core>("Core module");
    registerComponents(ecs);

    ecs.system<const TransformData, const WorldTransformData*, WorldTransformData>("World transform updating")
        .term_at(1).parent().cascade()
        .each([](flecs::entity e, const TransformData& local, const WorldTransformData* parent, WorldTransformData& world){});
}
