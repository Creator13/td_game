#include <flecs.h>
#include <catch2/catch_all.hpp>

#include "core/Transform.h"

#define ENGINE_TEST_TAG "[engine]"

using namespace core;

TEST_CASE("")
{
    flecs::world world;
    TransformSystem t(&world);

    auto e = t.placeEntity();
    CHECK(e.has<ecs::TransformHandle>());
}