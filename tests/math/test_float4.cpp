#include <catch2/catch_all.hpp>

#include "math/vec3.h"
#include "math/vec4.h"

using math::vec4;

namespace math
{
    std::ostream& operator<<(std::ostream& os, const vec4& rhs)
    {
        return os << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ", " << rhs.w << ")";
    }
}

TEST_CASE("vec4 construction", "[math][vector]")
{
    SECTION("Parameter constructor")
    {
        float x = 3.3;
        float y = -53.4;
        float z = 2.7;
        float w = 5.1;
        vec4 vec(x, y, z, w);
        REQUIRE(vec.x == x);
        REQUIRE(vec.y == y);
        REQUIRE(vec.z == z);
        REQUIRE(vec.w == w);
    }

    SECTION("Default constructor")
    {
        vec4 vec = vec4();
        REQUIRE(vec.x == 0);
        REQUIRE(vec.y == 0);
        REQUIRE(vec.z == 0);
        REQUIRE(vec.w == 0);
    }

    SECTION("vec3 constructor")
    {
        math::vec3 vec3 = math::vec3(3, 4, 5);
        vec4 vec = vec4(vec3, 1);
        REQUIRE(vec.x == 3);
        REQUIRE(vec.y == 4);
        REQUIRE(vec.z == 5);
        REQUIRE(vec.w == 1);
    }
}

TEST_CASE("vec4 add/subtract", "[math][vector]")
{
    vec4 a = vec4(2.3, 6.8, 1.0, 3.5);
    vec4 b = GENERATE(
        vec4(-1.2f, 4.3f, 3.0, 1.5),
        vec4(0, 0, 0, 0),
        vec4(3, 4, 2.5, 5.0),
        vec4(1, -.5f, -3.2f, 2.0)
    );

    SECTION("Addition")
    {
        vec4 result = a + b;
        REQUIRE(result.x == a.x + b.x);
        REQUIRE(result.y == a.y + b.y);
        REQUIRE(result.z == a.z + b.z);
        REQUIRE(result.w == a.w + b.w);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
        REQUIRE(a.z == 1.0f);
        REQUIRE(a.w == 3.5f);
    }

    SECTION("Subtraction")
    {
        vec4 result = a - b;
        REQUIRE(result.x == a.x - b.x);
        REQUIRE(result.y == a.y - b.y);
        REQUIRE(result.z == a.z - b.z);
        REQUIRE(result.w == a.w - b.w);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
        REQUIRE(a.z == 1.0f);
        REQUIRE(a.w == 3.5f);
    }

    SECTION("Commutative")
    {
        REQUIRE(a + b == b + a);
        REQUIRE(a - b != b - a);
    }

    SECTION("Assignment operators")
    {
        vec4 c = vec4(5.2f, 4.5f, -2.0f, 4.5);

        SECTION("Addition")
        {
            c += b;
            REQUIRE(c.x == 5.2f + b.x);
            REQUIRE(c.y == 4.5f + b.y);
            REQUIRE(c.z == -2.0f + b.z);
            REQUIRE(c.w == 4.5f + b.w);
        }

        SECTION("Subtraction")
        {
            c -= b;
            REQUIRE(c.x == 5.2f - b.x);
            REQUIRE(c.y == 4.5f - b.y);
            REQUIRE(c.z == -2.0f - b.z);
            REQUIRE(c.w == 4.5f - b.w);
        }
    }
}

TEST_CASE("vec4 scalar math", "[math][vector]")
{
    SECTION("Zero vector multiplication")
    {
        vec4 vec = vec4();
        vec4 result = vec * 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(result.w == 0);
        REQUIRE(vec == vec4());
    }

    SECTION("Zero scalar multiplication")
    {
        vec4 vec = vec4(7.45f, -0.54f, 3.0f, 1.5);
        vec4 result = vec * 0.f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(result.w == 0);
        REQUIRE(vec.x == 7.45f);
        REQUIRE(vec.y == -0.54f);
        REQUIRE(vec.z == 3.0f);
        REQUIRE(vec.w == 1.5f);
    }

    SECTION("Zero vector division")
    {
        vec4 vec = vec4();
        vec4 result = vec / 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(result.w == 0);
        REQUIRE(vec == vec4());
    }

    SECTION("Zero scalar division")
    {
        vec4 vec = vec4(7.45f, -0.54f, 3.0f, 1.5);
        vec4 result = vec / 0.f;
        REQUIRE(std::isinf(result.x));
        REQUIRE(std::isinf(result.y));
        REQUIRE(std::isinf(result.z));
        REQUIRE(std::isinf(result.w));
    }

    SECTION("Commutative multiplication")
    {
        vec4 vec = vec4(5.f, 6.3f, -1.2f, 0.f);
        float scalar = 3.45;
        REQUIRE(vec * scalar == scalar * vec);
    }
}

TEST_CASE("vec4 length", "[math][vector]")
{
    using pair = std::pair<vec4, float>;

    SECTION("Square length")
    {
        auto [vec, expected] = GENERATE(
            pair(vec4(0, 0, 0, 0), 0.f),
            pair(vec4(1, 0, 0, 0), 1.f),
            pair(vec4(0, 1, 0, 0), 1.f),
            pair(vec4(0, 0, 1, 0), 1.f),
            pair(vec4(0, 0, 0, 1), 1.f),
            pair(vec4(2, 2, 2, 2), 16.f),
            pair(vec4(5, 0, 0, 0), 25.f),
            pair(vec4(-3, -2, 1, 2), 18.f),
            pair(vec4(2, -3, 4, 5), 54.f),
            pair(vec4(-1, 0, 0, 0), 1.f)
        );
        REQUIRE(vec.sqrLength() == expected);
    }

    SECTION("Length")
    {
        auto [vec, expected] = GENERATE(
            pair(vec4(0, 0, 0, 0), 0.f),
            pair(vec4(1, 0, 0, 0), 1.f),
            pair(vec4(0, 1, 0, 0), 1.f),
            pair(vec4(0, 0, 1, 0), 1.f),
            pair(vec4(0, 0, 0, 1), 1.f),
            pair(vec4(2, 2, 2, 2), math::sqrt(16.f)),
            pair(vec4(5, 0, 0, 0), 5.f),
            pair(vec4(-3, -2, 1, 2), math::sqrt(18.f)),
            pair(vec4(3, 4, 0, 0), 5.f),
            pair(vec4(-1, 0, 0, 0), 1.f)
        );
        REQUIRE(vec.length() == expected);
    }
}

TEST_CASE("vec4 equality and inequality", "[math][vector]")
{
    vec4 a(1.f, 2.f, 3.f, 4.f);
    vec4 b(1.f, 2.f, 3.f, 4.f);
    vec4 c(2.f, 1.f, 3.f, 4.f);
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("vec4 unary minus", "[math][vector]")
{
    vec4 a(3.5f, -7.1f, 2.0f, -4.0f);
    vec4 b = -a;
    REQUIRE(b.x == -3.5f);
    REQUIRE(b.y == 7.1f);
    REQUIRE(b.z == -2.0f);
    REQUIRE(b.w == 4.0f);
    // Double negation returns the original
    REQUIRE(-b == a);
}

TEST_CASE("vec4 scalar division and multiplication symmetry", "[math][vector]")
{
    vec4 a(2.f, -4.f, 6.f, -8.f);

    SECTION("Multiply then divide")
    {
        vec4 scaled = (a * 5.f) / 5.f;
        REQUIRE(scaled == a);
    }

    SECTION("Division by scalar")
    {
        vec4 result = a / 2.f;
        REQUIRE_THAT(result.x, Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(result.y, Catch::Matchers::WithinRel(-2.f));
        REQUIRE_THAT(result.z, Catch::Matchers::WithinRel(3.f));
        REQUIRE_THAT(result.w, Catch::Matchers::WithinRel(-4.f));
    }

    SECTION("Scalar on left side")
    {
        vec4 result = 2.f * a;
        REQUIRE(result.x == 4.f);
        REQUIRE(result.y == -8.f);
        REQUIRE(result.z == 12.f);
        REQUIRE(result.w == -16.f);
    }
}

TEST_CASE("vec4 normalize", "[math][vector]")
{
    SECTION("Unit vectors remain unchanged")
    {
        vec4 right(1.f, 0.f, 0.f, 0.f);
        vec4 up(0.f, 1.f, 0.f, 0.f);
        vec4 forward(0.f, 0.f, 1.f, 0.f);
        vec4 time(0.f, 0.f, 0.f, 1.f);

        REQUIRE(normalize(right) == right);
        REQUIRE(normalize(up) == up);
        REQUIRE(normalize(forward) == forward);
        REQUIRE(normalize(time) == time);
    }

    SECTION("General vector normalization")
    {
        vec4 v(3.f, 4.f, 0.f, 0.f);
        vec4 n = normalize(v);

        REQUIRE_THAT(n.length(), Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(n.x, Catch::Matchers::WithinRel(0.6f));
        REQUIRE_THAT(n.y, Catch::Matchers::WithinRel(0.8f));
        REQUIRE_THAT(n.z, Catch::Matchers::WithinRel(0.f));
        REQUIRE_THAT(n.w, Catch::Matchers::WithinRel(0.f));
    }

    SECTION("Zero vector normalization")
    {
        vec4 zero = vec4::zero;
        vec4 n = normalize(zero);

        REQUIRE(n.x == 0);
        REQUIRE(n.y == 0);
        REQUIRE(n.z == 0);
        REQUIRE(n.w == 0);
    }
}

TEST_CASE("vec4 dot product", "[math][vector]")
{
    vec4 a(2.f, 3.f, 1.f, 0.5f);
    vec4 b(-1.f, 4.f, 2.f, 0.1f);
    CHECK_THAT(dot(a, b), Catch::Matchers::WithinRel(12.05f));

    a = vec4(1, 2, 3, -4);
    b = vec4(-1.5, 2, 1.2, 2.5);
    CHECK_THAT(dot(a, b), Catch::Matchers::WithinAbs(-3.9, 1e-6f));
}

TEST_CASE("vec4 static constants", "[math][vector]")
{
    REQUIRE(vec4::zero == vec4(0.f, 0.f, 0.f, 0.f));
    REQUIRE(vec4::one == vec4(1.f, 1.f, 1.f, 1.f));
}
