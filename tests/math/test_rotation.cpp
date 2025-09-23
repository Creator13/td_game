#include <catch2/catch_all.hpp>

#include "math/func.h"
#include "math/rotation.h"

using math::vec3;
using math::vec4;
using math::rot3x3;
using math::quaternion;

#define ROTATION_TEST_TAG "[math][rotation]"

TEST_CASE("Quaternion constructor", ROTATION_TEST_TAG)
{
    SECTION("Parameter constructor")
    {
        // The parameter constructor is not supposed to be used directly but is very useful in math code. Math code should always produce valid
        // quaternions. Placing restrictions on this constructor would only introduce unnecessary overhead. Any input should not yield any
        // errors, regardless of whether it represents a valid rotation/unit quaternion.
        float x = GENERATE(-1.2f, 4.f, 0, -1, 1);
        float y = GENERATE(2.5f, 28, 0, 1);
        float z = GENERATE(.5f, 0, 1);
        float w = GENERATE(2.f, 0, 1);

        quaternion q = quaternion(x, y, z, w);
        CHECK(q.x == x);
        CHECK(q.y == y);
        CHECK(q.z == z);
        CHECK(q.w == w);
    }

    SECTION("Default constructor")
    {
        // The default constructor should create an "uninitialized" instance of a quaternion, ie all zeroes.
        quaternion q = quaternion();
        CHECK(q.x == 0);
        CHECK(q.y == 0);
        CHECK(q.z == 0);
        CHECK(q.w == 0);
    }
}

TEST_CASE("Rotation matrix constructor", ROTATION_TEST_TAG)
{
    SECTION("Default constructor")
    {
        rot3x3 rotM = rot3x3();
        CHECK(rotM.xBasis == vec3::zero);
        CHECK(rotM.yBasis == vec3::zero);
        CHECK(rotM.zBasis == vec3::zero);
    }

    SECTION("Basis constructor")
    {
        // NOTE this constructs a matrix with invalid rotation, but the constructor should not mind this.
        vec3 x = vec3(3.4f, 1.2f, -.32f);
        vec3 y = vec3(-3.f, .2f, 1.f);
        vec3 z = vec3(.4f, -1.f, .2f);
        rot3x3 rotM = rot3x3(x, y, z);

        CHECK(rotM.xBasis == x);
        CHECK(rotM.yBasis == y);
        CHECK(rotM.zBasis == z);
    }

    SECTION("Value constructor")
    {
        // NOTE this constructs a matrix with invalid rotation, but the constructor should not mind this.
        rot3x3 rotM = rot3x3(
            1, 2, 3,
            4, 5, 6,
            7, 8, 9);
        CHECK(rotM.xBasis == vec3(1, 4, 7));
        CHECK(rotM.yBasis == vec3(2, 5, 8));
        CHECK(rotM.zBasis == vec3(3, 6, 9));
    }
}

TEST_CASE("Identity rotation", ROTATION_TEST_TAG)
{
    quaternion idtQ = quaternion::identity;
    SECTION("Normalize identity quaternion returns identity")
    {
        idtQ = normalize(idtQ);
        CHECK(idtQ == quaternion::identity);
    }

    rot3x3 idtM = rot3x3::identity;
    SECTION("Orthonormalize identity matrix returns identity")
    {
        idtM = orthonormalize(idtM);
        CHECK(idtM == rot3x3::identity);
    }

    vec3 vector(3, -2, 7);
    SECTION("Rotating vector with identity has no effect")
    {
        CHECK(rotate(idtQ, vector) == vector);
        CHECK(idtM * vector == vector);
    }
}

TEST_CASE("Quaternion normalize", ROTATION_TEST_TAG)
{
    quaternion q = quaternion(1, 2, 3, 4);
    q = normalize(q);
    float sqrMag = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    CHECK(math::approx(sqrMag, 1));
}
