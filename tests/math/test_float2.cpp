#include <catch2/catch_all.hpp>

#include "math/vec.h"

using math::float2;
using math::float3;
using math::float4;

namespace math {
    std::ostream& operator<<(std::ostream& os, const float2& rhs) {
        return os << "(" << rhs.x << ", " << rhs.y << ")";
    }

    std::ostream& operator<<(std::ostream& os, const float3& rhs) {
        return os << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
    }

    std::ostream& operator<<(std::ostream& os, const float4& rhs) {
        return os << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ", " << rhs.w << ")";
    }
}

TEST_CASE("float2 add/subtract", "[math]") {
    float2 a = float2(2.3, 6.8);
    float2 b = GENERATE(
        float2(-1.2f, 4.3f),
        float2(0, 0),
        float2(3, 4),
        float2(1, -.5f)
    );

    SECTION("Addition") {
        float2 result = a + b;
        REQUIRE(result.x == a.x + b.x);
        REQUIRE(result.y == a.y + b.y);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
    }

    SECTION("Subtraction") {
        float2 result = a - b;
        REQUIRE(result.x == a.x - b.x);
        REQUIRE(result.y == a.y - b.y);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
    }

    SECTION("Properties") {
        REQUIRE(a + b == b + a);
        REQUIRE(a - b != b - a);
    }
}

TEST_CASE("float2 length") {
    using pair = std::pair<float2, float>;

    SECTION("Square length") {
        auto [vec, expected] = GENERATE(
            pair(float2(0, 0), 0.f),
            pair(float2(1, 0), 1.f),
            pair(float2(0, 1), 1.f),
            pair(float2(2, 2), 8.f),
            pair(float2(5, 0), 25.f),
            pair(float2(-3, -2), 13.f),
            pair(float2(2, -3), 13.f),
            pair(float2(-1, 0), 1.f)
        );

        REQUIRE(vec.sqrLength() == expected);
    }

    SECTION("Length") {
        auto [vec, expected] = GENERATE(
            pair(float2(0, 0), 0.f),
            pair(float2(1, 0), 1.f),
            pair(float2(0, 1), 1.f),
            pair(float2(2, 2), math::sqrt(8.f)),
            pair(float2(5, 0), 5.f),
            pair(float2(-3, -2), math::sqrt(13.f)),
            pair(float2(4, -3), 5.f),
            pair(float2(-1, 0), 1.f)
        );

        REQUIRE(vec.length() == expected);
    }

    SECTION("Distance") {
        // Distance from zero to a vector is equal to the length of that vector
        float2 a = float2(3, 7);
        REQUIRE(distance(float2::zero, a) == a.length());

        // Distance from a to b is equal to the length of the difference vector between a and b
        float2 b = float2(13.4, 2);
        REQUIRE(distance(a, b) == (b - a).length());

        // Distance from a to b is equal to distance from b to a
        REQUIRE(distance(a, b) == distance(b, a));
    }
}
