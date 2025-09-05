#include <catch2/catch_all.hpp>

#include "math/vec.h"

using math::float3;

namespace math {
    std::ostream& operator<<(std::ostream& os, const float3& rhs) {
        return os << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
    }
}

TEST_CASE("float3 construction", "[math]") {
    SECTION("Parameter constructor") {
        float x = 3.3;
        float y = -53.4;
        float z = 2.7;
        float3 vec(x, y, z);

        REQUIRE(vec.x == 3.3f);
        REQUIRE(vec.y == -53.4f);
        REQUIRE(vec.z == 2.7f);
    }

    SECTION("Default constructor") {
        float3 vec = float3();
        REQUIRE(vec.x == 0);
        REQUIRE(vec.y == 0);
        REQUIRE(vec.z == 0);
    }
}

TEST_CASE("float3 add/subtract", "[math]") {
    float3 a = float3(2.3, 6.8, 1.0);
    float3 b = GENERATE(
        float3(-1.2f, 4.3f, 3.0),
        float3(0, 0, 0),
        float3(3, 4, 2.5),
        float3(1, -.5f, -3.2f)
    );

    SECTION("Addition") {
        float3 result = a + b;
        REQUIRE(result.x == a.x + b.x);
        REQUIRE(result.y == a.y + b.y);
        REQUIRE(result.z == a.z + b.z);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
        REQUIRE(a.z == 1.0f);
    }

    SECTION("Subtraction") {
        float3 result = a - b;
        REQUIRE(result.x == a.x - b.x);
        REQUIRE(result.y == a.y - b.y);
        REQUIRE(result.z == a.z - b.z);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
        REQUIRE(a.z == 1.0f);
    }

    SECTION("Commutative") {
        REQUIRE(a + b == b + a);
        REQUIRE(a - b != b - a);
    }

    SECTION("Assignment operators") {
        float3 c = float3(5.2f, 4.5f, -2.0f);

        SECTION("Addition") {
            c += b;
            REQUIRE(c.x == 5.2f + b.x);
            REQUIRE(c.y == 4.5f + b.y);
            REQUIRE(c.z == -2.0f + b.z);
        }

        SECTION("Subtraction") {
            c -= b;
            REQUIRE(c.x == 5.2f - b.x);
            REQUIRE(c.y == 4.5f - b.y);
            REQUIRE(c.z == -2.0f - b.z);
        }
    }
}

TEST_CASE("float3 scalar math", "[math]") {
    SECTION("Zero vector multiplication") {
        float3 vec = float3();
        float3 result = vec * 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(vec == float3());
    }

    SECTION("Zero scalar multiplication") {
        float3 vec = float3(7.45f, -0.54f, 3.0f);
        float3 result = vec * 0.f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(vec.x == 7.45f);
        REQUIRE(vec.y == -0.54f);
        REQUIRE(vec.z == 3.0f);
    }

    SECTION("Zero vector division") {
        float3 vec = float3();
        float3 result = vec / 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(vec == float3());
    }

    SECTION("Zero scalar division") {
        float3 vec = float3(7.45f, -0.54f, 3.0f);
        float3 result = vec / 0.f;
        REQUIRE(std::isinf(result.x));
        REQUIRE(std::isinf(result.y));
        REQUIRE(std::isinf(result.z));
    }
}

TEST_CASE("float3 length", "[math]") {
    using pair = std::pair<float3, float>;
    SECTION("Square length") {
        auto [vec, expected] = GENERATE(
            pair(float3(0, 0, 0), 0.f),
            pair(float3(1, 0, 0), 1.f),
            pair(float3(0, 1, 0), 1.f),
            pair(float3(0, 0, 1), 1.f),
            pair(float3(2, 2, 2), 12.f),
            pair(float3(5, 0, 0), 25.f),
            pair(float3(-3, -2, 1), 14.f),
            pair(float3(2, -3, 4), 29.f),
            pair(float3(-1, 0, 0), 1.f)
        );
        REQUIRE(vec.sqrLength() == expected);
    }
    SECTION("Length") {
        auto [vec, expected] = GENERATE(
            pair(float3(0, 0, 0), 0.f),
            pair(float3(1, 0, 0), 1.f),
            pair(float3(0, 1, 0), 1.f),
            pair(float3(0, 0, 1), 1.f),
            pair(float3(2, 2, 2), math::sqrt(12.f)),
            pair(float3(5, 0, 0), 5.f),
            pair(float3(-3, -2, 1), math::sqrt(14.f)),
            pair(float3(4, -3, 0), 5.f),
            pair(float3(-1, 0, 0), 1.f)
        );
        REQUIRE(vec.length() == expected);
    }

    SECTION("Distance") {
        // Distance from zero to a vector is equal to the length of that vector
        float3 a = float3(3, 7, 2);
        REQUIRE(distance(float3::zero, a) == a.length());

        // Distance from a to b is equal to the length of the difference vector between a and b
        float3 b = float3(13.4, 2, 5);
        REQUIRE(distance(a, b) == (b - a).length());

        // Distance from a to b is equal to distance from b to a
        REQUIRE(distance(a, b) == distance(b, a));
    }
}

TEST_CASE("float3 equality and inequality", "[math]") {
    float3 a(1.f, 2.f, 3.f);
    float3 b(1.f, 2.f, 3.f);
    float3 c(2.f, 1.f, 3.f);

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("float3 unary minus", "[math]") {
    float3 a(3.5f, -7.1f, 2.0f);
    float3 b = -a;

    REQUIRE(b.x == -3.5f);
    REQUIRE(b.y == 7.1f);
    REQUIRE(b.z == -2.0f);

    REQUIRE(-b == a);
}

TEST_CASE("float3 scalar division and multiplication symmetry", "[math]") {
    float3 a(2.f, -4.f, 6.f);
    SECTION("Multiply then divide") {
        float3 scaled = (a * 5.f) / 5.f;
        REQUIRE(scaled == a);
    }
    SECTION("Division by scalar") {
        float3 result = a / 2.f;
        REQUIRE_THAT(result.x, Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(result.y, Catch::Matchers::WithinRel(-2.f));
        REQUIRE_THAT(result.z, Catch::Matchers::WithinRel(3.f));
    }
    SECTION("Scalar on left side") {
        float3 result = 2.f * a;
        REQUIRE(result.x == 4.f);
        REQUIRE(result.y == -8.f);
        REQUIRE(result.z == 12.f);
    }
}

TEST_CASE("float3 normalize", "[math]") {
    SECTION("Unit vectors remain unchanged") {
        REQUIRE(normalize(float3::right) == float3::right);
        REQUIRE(normalize(float3::up) == float3::up);
        REQUIRE(normalize(float3::backward) == float3::backward);
    }

    SECTION("General vector normalization") {
        float3 v(3.f, 4.f, 0.f);
        float3 n = normalize(v);
        REQUIRE_THAT(n.length(), Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(n.x, Catch::Matchers::WithinRel(0.6f));
        REQUIRE_THAT(n.y, Catch::Matchers::WithinRel(0.8f));
        REQUIRE_THAT(n.z, Catch::Matchers::WithinRel(0.f));
    }

    SECTION("Zero vector normalization") {
        float3 zero = float3::zero;
        float3 n = normalize(zero);
        REQUIRE(std::isnan(n.x));
        REQUIRE(std::isnan(n.y));
        REQUIRE(std::isnan(n.z));
    }
}

TEST_CASE("float3 dot product", "[math]") {
    SECTION("Orthogonal vectors") {
        REQUIRE(dot(float3::right, float3::up) == 0.f);
        REQUIRE(dot(float3::up, float3::backward) == 0.f);
        REQUIRE(dot(float3::backward, float3::right) == 0.f);
    }

    SECTION("General vectors") {
        float3 a = float3(2.f, 3.f, 1.f);
        float3 b = float3(-1.f, 4.f, 2.f);
        REQUIRE_THAT(dot(a, b), Catch::Matchers::WithinRel(12.f));
    }
}

TEST_CASE("float3 cross product", "[math]") {
    SECTION("Cross product of unit vectors") {
        // Cross product of two unit vectors should give another unit vector
        REQUIRE(cross(float3::right, float3::forward) == float3::up);
        REQUIRE(cross(float3::forward, float3::right) == float3::down);
        REQUIRE(cross(float3::forward, float3::up) == float3::right);
        REQUIRE(cross(float3::up, float3::forward) == float3::left);
        REQUIRE(cross(float3::up, float3::right) == float3::forward);
        REQUIRE(cross(float3::right, float3::up) == float3::backward);
    }

    SECTION("Cross product properties") {
        float3 a(1.f, 0.f, 0.f);
        float3 b(0.f, 1.f, 0.f);
        // Anticommutativity
        REQUIRE(cross(a, b) == -cross(b, a));
        // Cross product with self is zero vector
        REQUIRE(cross(a, a) == float3::zero);
    }

    SECTION("General vectors") {
        float3 a(2.f, 3.f, 4.f);
        float3 b(5.f, 6.f, 7.f);
        float3 result = cross(a, b);
        float3 expected(-3.f, 6.f, -3.f); // a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x
        REQUIRE(result == expected);
    }
}

TEST_CASE("float3 static constants", "[math]") {
    REQUIRE(float3::zero == float3(0.f, 0.f, 0.f));
    REQUIRE(float3::one == float3(1.f, 1.f, 1.f));
    REQUIRE(float3::right == float3(1.0f, 0.0f, 0.0f));
    REQUIRE(float3::left == float3(-1.0f, 0.0f, 0.0f));
    REQUIRE(float3::forward == float3(0.0f, 1.0f, 0.0f));
    REQUIRE(float3::backward == float3(0.0f, -1.0f, 0.0f));
    REQUIRE(float3::up == float3(0.0f, 0.0f, 1.0f));
    REQUIRE(float3::down == float3(0.0f, 0.0f, -1.0f));
}
