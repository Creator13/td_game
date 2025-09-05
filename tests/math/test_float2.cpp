#include <catch2/catch_all.hpp>

#include "math/vec.h"

using math::float2;

namespace math {
    std::ostream& operator<<(std::ostream& os, const float2& rhs) {
        return os << "(" << rhs.x << ", " << rhs.y << ")";
    }
}

TEST_CASE("float2 construction", "[math]") {
    SECTION("Parameter constructor") {
        float x = 3.3;
        float y = -53.4;
        float2 vec(x, y);

        REQUIRE(vec.x == 3.3f);
        REQUIRE(vec.y == -53.4f);
    }

    SECTION("Default constructor") {
        float2 vec = float2();
        REQUIRE(vec.x == 0);
        REQUIRE(vec.y == 0);
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

    SECTION("Commutative") {
        REQUIRE(a + b == b + a);
        REQUIRE(a - b != b - a);
    }

    SECTION("Assignment operators") {
        float2 c = float2(5.2f, 4.5f);
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

TEST_CASE("float2 scalar math", "[math]") {
    SECTION("Zero vector multiplication") {
        float2 vec = float2();
        float2 result = vec * 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(vec == float2());
    }

    SECTION("Zero scalar multiplication") {
        float2 vec = float2(7.45f, -0.54f);
        float2 result = vec * 0.f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(vec.x == 7.45f);
        REQUIRE(vec.y == -0.54f);
    }

    SECTION("Zero vector division") {
        float2 vec = float2();
        float2 result = vec * 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(vec == float2());
    }

    SECTION("Zero scalar division") {
        float2 vec = float2(7.45f, -0.54f);
        float2 result = vec / 0.f;
        REQUIRE(std::isinf(result.x));
        REQUIRE(std::isinf(result.y));
    }
}

TEST_CASE("float2 length", "[math]") {
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

TEST_CASE("float2 equality and inequality", "[math]") {
    float2 a(1.f, 2.f);
    float2 b(1.f, 2.f);
    float2 c(2.f, 1.f);

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("float2 unary minus", "[math]") {
    float2 a(3.5f, -7.1f);
    float2 b = -a;

    REQUIRE(b.x == -3.5f);
    REQUIRE(b.y == 7.1f);

    // Double negation returns the original
    REQUIRE(-b == a);
}

TEST_CASE("float2 scalar division and multiplication symmetry", "[math]") {
    float2 a(2.f, -4.f);

    SECTION("Multiply then divide") {
        float2 scaled = (a * 5.f) / 5.f;
        REQUIRE(scaled == a);
    }

    SECTION("Division by scalar") {
        float2 result = a / 2.f;
        REQUIRE_THAT(result.x, Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(result.y, Catch::Matchers::WithinRel(-2.f));
    }

    SECTION("Scalar on left side") {
        float2 result = 2.f * a;
        REQUIRE(result.x == 4.f);
        REQUIRE(result.y == -8.f);
    }
}

TEST_CASE("float2 normalize", "[math]") {
    SECTION("Unit vectors remain unchanged") {
        REQUIRE(normalize(float2::right) == float2::right);
        REQUIRE(normalize(float2::up) == float2::up);
    }

    SECTION("General vector normalization") {
        float2 v(3.f, 4.f);
        float2 n = normalize(v);

        REQUIRE_THAT(n.length(), Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(n.x, Catch::Matchers::WithinRel(0.6f));
        REQUIRE_THAT(n.y, Catch::Matchers::WithinRel(0.8f));
    }

    SECTION("Zero vector normalization") {
        float2 zero = float2::zero;
        float2 n = normalize(zero);
        REQUIRE(std::isnan(n.x));
        REQUIRE(std::isnan(n.y));
    }
}

TEST_CASE("float2 dot product", "[math]") {
    SECTION("Orthogonal vectors") {
        REQUIRE(dot(float2::right, float2::up) == 0.f);
    }

    SECTION("General vectors") {
        float2 a(2.f, 3.f);
        float2 b(-1.f, 4.f);
        REQUIRE_THAT(dot(a, b), Catch::Matchers::WithinRel(10.f)); // 2*(-1) + 3*4 = -2 + 12 = 10
    }
}

TEST_CASE("float2 perp", "[math]") {
    float2 a(2.f, 3.f);
    float2 p = perp(a);

    REQUIRE(p == float2(3.f, -2.f));

    // Perpendicular vector is orthogonal
    REQUIRE_THAT(dot(a, p), Catch::Matchers::WithinRel(0.f));
}

TEST_CASE("float2 static constants", "[math]") {
    REQUIRE(float2::zero == float2(0.f, 0.f));
    REQUIRE(float2::one == float2(1.f, 1.f));
    REQUIRE(float2::up == float2(0.f, 1.f));
    REQUIRE(float2::down == float2(0.f, -1.f));
    REQUIRE(float2::left == float2(-1.f, 0.f));
    REQUIRE(float2::right == float2(1.f, 0.f));
}
