#include <catch2/catch_all.hpp>

#include "math/func.h"

TEST_CASE("Lerp") {
    // Normal usage
    REQUIRE(math::lerp(2.f, 5.f, 1.f) == 5.f);
    REQUIRE(math::lerp(-2.f, 5.f, 0.f) == -2.f);
    REQUIRE(math::lerp(-2.f, 6.f, .5f) == 2.f);
    REQUIRE(math::lerp(-2.f, 6.f, .25f) == 0.f);
    REQUIRE(math::lerp(-2.f, 6.f, .75f) == 4.f);

    // Edge case 0
    REQUIRE(math::lerp(0, 0, 0) == 0);

    // Edge case equals
    REQUIRE(math::lerp(23.7f, 23.7f, .3f) == 23.7f);
}