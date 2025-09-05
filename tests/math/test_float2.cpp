#include <catch2/catch_all.hpp>

#include "math/vec2.h"

using math::vec2;

namespace math {
    std::ostream& operator<<(std::ostream& os, const vec2& rhs) {
        return os << "(" << rhs.x << ", " << rhs.y << ")";
    }
}

TEST_CASE("float2 construction", "[math][vector]") {
    SECTION("Parameter constructor") {
        float x = 3.3;
        float y = -53.4;
        vec2 vec(x, y);

        REQUIRE(vec.x == x);
        REQUIRE(vec.y == y);
    }

    SECTION("Default constructor") {
        vec2 vec = vec2();
        REQUIRE(vec.x == 0);
        REQUIRE(vec.y == 0);
    }
}

TEST_CASE("float2 add/subtract", "[math][vector]") {
    vec2 a = vec2(2.3, 6.8);
    vec2 b = GENERATE(
        vec2(-1.2f, 4.3f),
        vec2(0, 0),
        vec2(3, 4),
        vec2(1, -.5f)
    );

    SECTION("Addition") {
        vec2 result = a + b;
        REQUIRE(result.x == a.x + b.x);
        REQUIRE(result.y == a.y + b.y);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
    }

    SECTION("Subtraction") {
        vec2 result = a - b;
        REQUIRE(result.x == a.x - b.x);
        REQUIRE(result.y == a.y - b.y);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
    }

    SECTION("Commutative") {
        REQUIRE(a + b == b + a);
        REQUIRE(a - b != b - a);
    }

    SECTION("Assignment operators") {
        vec2 c = vec2(5.2f, 4.5f);
        SECTION("Addition") {
            c += b;
            REQUIRE(c.x == 5.2f + b.x);
            REQUIRE(c.y == 4.5f + b.y);
        }
        SECTION("Subtraction") {
            c -= b;
            REQUIRE(c.x == 5.2f - b.x);
            REQUIRE(c.y == 4.5f - b.y);
        }
    }
}

TEST_CASE("float2 scalar math", "[math][vector]") {
    SECTION("Zero vector multiplication") {
        vec2 vec = vec2();
        vec2 result = vec * 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(vec == vec2());
    }

    SECTION("Zero scalar multiplication") {
        vec2 vec = vec2(7.45f, -0.54f);
        vec2 result = vec * 0.f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(vec.x == 7.45f);
        REQUIRE(vec.y == -0.54f);
    }

    SECTION("Zero vector division") {
        vec2 vec = vec2();
        vec2 result = vec * 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(vec == vec2());
    }

    SECTION("Zero scalar division") {
        vec2 vec = vec2(7.45f, -0.54f);
        vec2 result = vec / 0.f;
        REQUIRE(std::isinf(result.x));
        REQUIRE(std::isinf(result.y));
    }

    SECTION("Commutative multiplication") {
        vec2 vec = vec2(5.f, 6.3f);
        float scalar = 3.45;
        REQUIRE(vec * scalar == scalar * vec);
    }
}

TEST_CASE("float2 length", "[math][vector]") {
    using pair = std::pair<vec2, float>;

    SECTION("Square length") {
        auto [vec, expected] = GENERATE(
            pair(vec2(0, 0), 0.f),
            pair(vec2(1, 0), 1.f),
            pair(vec2(0, 1), 1.f),
            pair(vec2(2, 2), 8.f),
            pair(vec2(5, 0), 25.f),
            pair(vec2(-3, -2), 13.f),
            pair(vec2(2, -3), 13.f),
            pair(vec2(-1, 0), 1.f)
        );

        REQUIRE(vec.sqrLength() == expected);
    }

    SECTION("Length") {
        auto [vec, expected] = GENERATE(
            pair(vec2(0, 0), 0.f),
            pair(vec2(1, 0), 1.f),
            pair(vec2(0, 1), 1.f),
            pair(vec2(2, 2), math::sqrt(8.f)),
            pair(vec2(5, 0), 5.f),
            pair(vec2(-3, -2), math::sqrt(13.f)),
            pair(vec2(4, -3), 5.f),
            pair(vec2(-1, 0), 1.f)
        );

        REQUIRE(vec.length() == expected);
    }

    SECTION("Distance") {
        // Distance from zero to a vector is equal to the length of that vector
        vec2 a = vec2(3, 7);
        REQUIRE(distance(vec2::zero, a) == a.length());

        // Distance from a to b is equal to the length of the difference vector between a and b
        vec2 b = vec2(13.4, 2);
        REQUIRE(distance(a, b) == (b - a).length());

        // Distance from a to b is equal to distance from b to a
        REQUIRE(distance(a, b) == distance(b, a));
    }
}

TEST_CASE("float2 equality and inequality", "[math][vector]") {
    vec2 a(1.f, 2.f);
    vec2 b(1.f, 2.f);
    vec2 c(2.f, 1.f);

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("float2 unary minus", "[math][vector]") {
    vec2 a(3.5f, -7.1f);
    vec2 b = -a;

    REQUIRE(b.x == -3.5f);
    REQUIRE(b.y == 7.1f);

    // Double negation returns the original
    REQUIRE(-b == a);
}

TEST_CASE("float2 scalar division and multiplication symmetry", "[math][vector]") {
    vec2 a(2.f, -4.f);

    SECTION("Multiply then divide") {
        vec2 scaled = (a * 5.f) / 5.f;
        REQUIRE(scaled == a);
    }

    SECTION("Division by scalar") {
        vec2 result = a / 2.f;
        REQUIRE_THAT(result.x, Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(result.y, Catch::Matchers::WithinRel(-2.f));
    }

    SECTION("Scalar on left side") {
        vec2 result = 2.f * a;
        REQUIRE(result.x == 4.f);
        REQUIRE(result.y == -8.f);
    }
}

TEST_CASE("float2 normalize", "[math][vector]") {
    SECTION("Unit vectors remain unchanged") {
        REQUIRE(normalize(vec2::right) == vec2::right);
        REQUIRE(normalize(vec2::up) == vec2::up);
    }

    SECTION("General vector normalization") {
        vec2 v(3.f, 4.f);
        vec2 n = normalize(v);

        REQUIRE_THAT(n.length(), Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(n.x, Catch::Matchers::WithinRel(0.6f));
        REQUIRE_THAT(n.y, Catch::Matchers::WithinRel(0.8f));
    }

    SECTION("Zero vector normalization") {
        vec2 zero = vec2::zero;
        vec2 n = normalize(zero);
        REQUIRE(std::isnan(n.x));
        REQUIRE(std::isnan(n.y));
    }
}

TEST_CASE("float2 dot product", "[math][vector]") {
    SECTION("Orthogonal vectors") {
        REQUIRE(dot(vec2::right, vec2::up) == 0.f);
    }

    SECTION("General vectors") {
        vec2 a(2.f, 3.f);
        vec2 b(-1.f, 4.f);
        REQUIRE_THAT(dot(a, b), Catch::Matchers::WithinRel(10.f)); // 2*(-1) + 3*4 = -2 + 12 = 10
    }
}

TEST_CASE("float2 perp", "[math][vector]") {
    vec2 a(2.f, 3.f);
    vec2 p = perp(a);

    REQUIRE(p == vec2(3.f, -2.f));

    // Perpendicular vector is orthogonal
    REQUIRE_THAT(dot(a, p), Catch::Matchers::WithinRel(0.f));
}

TEST_CASE("float2 static constants", "[math][vector]") {
    REQUIRE(vec2::zero == vec2(0.f, 0.f));
    REQUIRE(vec2::one == vec2(1.f, 1.f));
    REQUIRE(vec2::up == vec2(0.f, 1.f));
    REQUIRE(vec2::down == vec2(0.f, -1.f));
    REQUIRE(vec2::left == vec2(-1.f, 0.f));
    REQUIRE(vec2::right == vec2(1.f, 0.f));
}
