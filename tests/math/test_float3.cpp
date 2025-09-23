#include <catch2/catch_all.hpp>

#include "math/vec3.h"

using math::vec3;

namespace math
{
    std::ostream& operator<<(std::ostream& os, const vec3& rhs)
    {
        return os << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
    }
}

TEST_CASE("float3 construction", "[math][vector]")
{
    SECTION("Parameter constructor")
    {
        float x = 3.3;
        float y = -53.4;
        float z = 2.7;
        vec3 vec(x, y, z);

        REQUIRE(vec.x == x);
        REQUIRE(vec.y == y);
        REQUIRE(vec.z == z);
    }

    SECTION("Default constructor")
    {
        vec3 vec = vec3();
        REQUIRE(vec.x == 0);
        REQUIRE(vec.y == 0);
        REQUIRE(vec.z == 0);
    }
}

TEST_CASE("float3 add/subtract", "[math][vector]")
{
    vec3 a = vec3(2.3, 6.8, 1.0);
    vec3 b = GENERATE(
        vec3(-1.2f, 4.3f, 3.0),
        vec3(0, 0, 0),
        vec3(3, 4, 2.5),
        vec3(1, -.5f, -3.2f)
    );

    SECTION("Addition")
    {
        vec3 result = a + b;
        REQUIRE(result.x == a.x + b.x);
        REQUIRE(result.y == a.y + b.y);
        REQUIRE(result.z == a.z + b.z);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
        REQUIRE(a.z == 1.0f);
    }

    SECTION("Subtraction")
    {
        vec3 result = a - b;
        REQUIRE(result.x == a.x - b.x);
        REQUIRE(result.y == a.y - b.y);
        REQUIRE(result.z == a.z - b.z);
        REQUIRE(a.x == 2.3f);
        REQUIRE(a.y == 6.8f);
        REQUIRE(a.z == 1.0f);
    }

    SECTION("Commutative")
    {
        REQUIRE(a + b == b + a);
        REQUIRE(a - b != b - a);
    }

    SECTION("Assignment operators")
    {
        vec3 c = vec3(5.2f, 4.5f, -2.0f);

        SECTION("Addition")
        {
            c += b;
            REQUIRE(c.x == 5.2f + b.x);
            REQUIRE(c.y == 4.5f + b.y);
            REQUIRE(c.z == -2.0f + b.z);
        }

        SECTION("Subtraction")
        {
            c -= b;
            REQUIRE(c.x == 5.2f - b.x);
            REQUIRE(c.y == 4.5f - b.y);
            REQUIRE(c.z == -2.0f - b.z);
        }
    }
}

TEST_CASE("float3 scalar math", "[math][vector]")
{
    SECTION("Zero vector multiplication")
    {
        vec3 vec = vec3();
        vec3 result = vec * 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(vec == vec3());
    }

    SECTION("Zero scalar multiplication")
    {
        vec3 vec = vec3(7.45f, -0.54f, 3.0f);
        vec3 result = vec * 0.f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(vec.x == 7.45f);
        REQUIRE(vec.y == -0.54f);
        REQUIRE(vec.z == 3.0f);
    }

    SECTION("Zero vector division")
    {
        vec3 vec = vec3();
        vec3 result = vec / 4.5f;
        REQUIRE(result.x == 0);
        REQUIRE(result.y == 0);
        REQUIRE(result.z == 0);
        REQUIRE(vec == vec3());
    }

    SECTION("Zero scalar division")
    {
        vec3 vec = vec3(7.45f, -0.54f, 3.0f);
        vec3 result = vec / 0.f;
        REQUIRE(std::isinf(result.x));
        REQUIRE(std::isinf(result.y));
        REQUIRE(std::isinf(result.z));
    }

    SECTION("Commutative multiplication")
    {
        vec3 vec = vec3(5.f, 6.3f, -1.2f);
        float scalar = 3.45;
        REQUIRE(vec * scalar == scalar * vec);
    }
}

TEST_CASE("float3 length", "[math][vector]")
{
    using pair = std::pair<vec3, float>;
    SECTION("Square length")
    {
        auto [vec, expected] = GENERATE(
            pair(vec3(0, 0, 0), 0.f),
            pair(vec3(1, 0, 0), 1.f),
            pair(vec3(0, 1, 0), 1.f),
            pair(vec3(0, 0, 1), 1.f),
            pair(vec3(2, 2, 2), 12.f),
            pair(vec3(5, 0, 0), 25.f),
            pair(vec3(-3, -2, 1), 14.f),
            pair(vec3(2, -3, 4), 29.f),
            pair(vec3(-1, 0, 0), 1.f)
        );
        REQUIRE(vec.sqrLength() == expected);
    }
    SECTION("Length")
    {
        auto [vec, expected] = GENERATE(
            pair(vec3(0, 0, 0), 0.f),
            pair(vec3(1, 0, 0), 1.f),
            pair(vec3(0, 1, 0), 1.f),
            pair(vec3(0, 0, 1), 1.f),
            pair(vec3(2, 2, 2), math::sqrt(12.f)),
            pair(vec3(5, 0, 0), 5.f),
            pair(vec3(-3, -2, 1), math::sqrt(14.f)),
            pair(vec3(4, -3, 0), 5.f),
            pair(vec3(-1, 0, 0), 1.f)
        );
        REQUIRE(vec.length() == expected);
    }

    SECTION("Distance")
    {
        // Distance from zero to a vector is equal to the length of that vector
        vec3 a = vec3(3, 7, 2);
        REQUIRE(distance(vec3::zero, a) == a.length());

        // Distance from a to b is equal to the length of the difference vector between a and b
        vec3 b = vec3(13.4, 2, 5);
        REQUIRE(distance(a, b) == (b - a).length());

        // Distance from a to b is equal to distance from b to a
        REQUIRE(distance(a, b) == distance(b, a));
    }
}

TEST_CASE("float3 equality and inequality", "[math][vector]")
{
    vec3 a(1.f, 2.f, 3.f);
    vec3 b(1.f, 2.f, 3.f);
    vec3 c(2.f, 1.f, 3.f);

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("float3 unary minus", "[math][vector]")
{
    vec3 a(3.5f, -7.1f, 2.0f);
    vec3 b = -a;

    REQUIRE(b.x == -3.5f);
    REQUIRE(b.y == 7.1f);
    REQUIRE(b.z == -2.0f);

    REQUIRE(-b == a);
}

TEST_CASE("float3 scalar division and multiplication symmetry", "[math][vector]")
{
    vec3 a(2.f, -4.f, 6.f);
    SECTION("Multiply then divide")
    {
        vec3 scaled = (a * 5.f) / 5.f;
        REQUIRE(scaled == a);
    }
    SECTION("Division by scalar")
    {
        vec3 result = a / 2.f;
        REQUIRE_THAT(result.x, Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(result.y, Catch::Matchers::WithinRel(-2.f));
        REQUIRE_THAT(result.z, Catch::Matchers::WithinRel(3.f));
    }
    SECTION("Scalar on left side")
    {
        vec3 result = 2.f * a;
        REQUIRE(result.x == 4.f);
        REQUIRE(result.y == -8.f);
        REQUIRE(result.z == 12.f);
    }
}

TEST_CASE("float3 normalize", "[math][vector]")
{
    SECTION("Unit vectors remain unchanged")
    {
        REQUIRE(normalize(vec3::right) == vec3::right);
        REQUIRE(normalize(vec3::up) == vec3::up);
        REQUIRE(normalize(vec3::back) == vec3::back);
    }

    SECTION("General vector normalization")
    {
        vec3 v(3.f, 4.f, 0.f);
        vec3 n = normalize(v);
        REQUIRE_THAT(n.length(), Catch::Matchers::WithinRel(1.f));
        REQUIRE_THAT(n.x, Catch::Matchers::WithinRel(0.6f));
        REQUIRE_THAT(n.y, Catch::Matchers::WithinRel(0.8f));
        REQUIRE_THAT(n.z, Catch::Matchers::WithinRel(0.f));
    }

    SECTION("Zero vector normalization")
    {
        vec3 zero = vec3::zero;
        vec3 n = normalize(zero);
        REQUIRE(n.x == 0);
        REQUIRE(n.y == 0);
        REQUIRE(n.z == 0);
    }
}

TEST_CASE("float3 dot product", "[math][vector]")
{
    SECTION("Orthogonal vectors")
    {
        REQUIRE(dot(vec3::right, vec3::up) == 0.f);
        REQUIRE(dot(vec3::up, vec3::back) == 0.f);
        REQUIRE(dot(vec3::back, vec3::right) == 0.f);
    }

    SECTION("General vectors")
    {
        vec3 a = vec3(2.f, 3.f, 1.f);
        vec3 b = vec3(-1.f, 4.f, 2.f);
        REQUIRE_THAT(dot(a, b), Catch::Matchers::WithinRel(12.f));
    }
}

TEST_CASE("float3 cross product", "[math][vector]")
{
    SECTION("Cross product of unit vectors")
    {
        // Cross product of two unit vectors should give another unit vector
        REQUIRE(cross(vec3::right, vec3::forward) == vec3::up);
        REQUIRE(cross(vec3::forward, vec3::right) == vec3::down);
        REQUIRE(cross(vec3::forward, vec3::up) == vec3::right);
        REQUIRE(cross(vec3::up, vec3::forward) == vec3::left);
        REQUIRE(cross(vec3::up, vec3::right) == vec3::forward);
        REQUIRE(cross(vec3::right, vec3::up) == vec3::back);
    }

    SECTION("Cross product properties")
    {
        vec3 a(1.f, 0.f, 0.f);
        vec3 b(0.f, 1.f, 0.f);
        // Anticommutativity
        REQUIRE(cross(a, b) == -cross(b, a));
        // Cross product with self is zero vector
        REQUIRE(cross(a, a) == vec3::zero);
    }

    SECTION("General vectors")
    {
        vec3 a(2.f, 3.f, 4.f);
        vec3 b(5.f, 6.f, 7.f);
        vec3 result = cross(a, b);
        vec3 expected(-3.f, 6.f, -3.f); // a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x
        REQUIRE(result == expected);
    }
}

TEST_CASE("float3 static constants", "[math][vector]")
{
    REQUIRE(vec3::zero == vec3(0.f, 0.f, 0.f));
    REQUIRE(vec3::one == vec3(1.f, 1.f, 1.f));
    REQUIRE(vec3::right == vec3(1.0f, 0.0f, 0.0f));
    REQUIRE(vec3::left == vec3(-1.0f, 0.0f, 0.0f));
    REQUIRE(vec3::forward == vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(vec3::back == vec3(0.0f, -1.0f, 0.0f));
    REQUIRE(vec3::up == vec3(0.0f, 0.0f, 1.0f));
    REQUIRE(vec3::down == vec3(0.0f, 0.0f, -1.0f));
}
