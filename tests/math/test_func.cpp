#include <catch2/catch_all.hpp>

#include "math/func.h"

#define FUNCTIONS_TEST_TAG "[math][function]"

TEST_CASE("Lerp", FUNCTIONS_TEST_TAG) {
    // Normal usage
    CHECK(math::lerp(2.f, 5.f, 1.f) == 5.f);
    CHECK(math::lerp(-2.f, 5.f, 0.f) == -2.f);
    CHECK(math::lerp(-2.f, 6.f, .5f) == 2.f);
    CHECK(math::lerp(-2.f, 6.f, .25f) == 0.f);
    CHECK(math::lerp(-2.f, 6.f, .75f) == 4.f);

    // Edge case 0
    CHECK(math::lerp(0, 0, 0) == 0);

    // Edge case equal
    CHECK(math::lerp(23.7f, 23.7f, .3f) == 23.7f);
}

TEST_CASE("Approx", FUNCTIONS_TEST_TAG) {
    SECTION("Default epsilon") {
        CHECK(math::approx(2, 2));
        CHECK(math::approx(0, 0));
        CHECK(math::approx(0.14, 0.1400001));
        CHECK(math::approx(0.1400001, 0.14));

        CHECK_FALSE(math::approx(0.1, 0));

        CHECK_FALSE(math::approx(0, 0.1));
    }

    SECTION("Custom epsilon") {
        CHECK(math::approx(1.1, 1.0, .11f));
        CHECK(math::approx(1.0, 1.1, .11f));
        CHECK_FALSE(math::approx(1.11, 1.0, .11f));
        CHECK_FALSE(math::approx(1.0, 1.11, .11f));
    }
}

TEST_CASE("Abs", FUNCTIONS_TEST_TAG) {
    CHECK(math::abs(-1) == 1);
    CHECK(math::abs(1) == 1);
    CHECK(math::abs(0) == 0);
    CHECK(math::abs(-0) == 0);
    CHECK(math::abs(-23.4223f) == 23.4223f);
    CHECK(math::abs(23.4223f) == 23.4223f);
    CHECK(math::abs(-2382739324.f) == 2382739324.f);
    CHECK(math::abs(-0.1563343234f) == 0.1563343234f);
    CHECK(math::abs(0.1563343234f) == 0.1563343234f);
}