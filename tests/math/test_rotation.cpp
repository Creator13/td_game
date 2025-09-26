#include <catch2/catch_all.hpp>

#include "math/vec.h"
#include "math/mat4.h"
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

TEST_CASE("Inverse rotation", ROTATION_TEST_TAG)
{
    vec3 vector = vec3(4, -5.98, 2.3f);

    SECTION("Inverse matrix (transpose)")
    {
        rot3x3 rot = rot3x3::eulerAngles(23, 45, 76);
        vec3 rotated = rot * vector;
        rot3x3 invRot = rot.getTranspose();
        CHECK(math::approx(invRot * rotated, vector));
    }

    SECTION("Inverse quaternion")
    {
        quaternion q = quaternion::eulerAngles(23, 45, 76);
        vec3 rotated = math::rotate(q, vector);
        quaternion qInv = inverse(q);
        CHECK(math::approx(math::rotate(qInv, rotated), vector));
    }
}

TEST_CASE("Euler angles", ROTATION_TEST_TAG)
{
    vec3 angles = vec3(63, 17, -40);

    SECTION("Quaternion euler angles")
    {
        quaternion q = quaternion::eulerAngles(angles);
        CHECK(math::approx(angles, toEuler(q)));
    }

    SECTION("Rot3x3 euler angles")
    {
        rot3x3 rot = rot3x3::eulerAngles(angles);
        CHECK(math::approx(angles, math::toEuler(rot)));
    }
}

TEST_CASE("Cardinal axes rotation", ROTATION_TEST_TAG)
{
    SECTION("Euler")
    {
        SECTION("Rotation matrix")
        {
            rot3x3 rot = rot3x3::eulerAngles(0, 0, 90);
            vec3 result = rot * vec3::right;
            REQUIRE(math::approx(result, vec3::forward));
        }

        SECTION("Quaternion")
        {
            quaternion rot = quaternion::eulerAngles(0, 0, 90);
            vec3 result = math::rotate(rot, vec3::right);
            REQUIRE(math::approx(result, vec3::forward));
        }
    }
}

TEST_CASE("Quaternion and matrix equivalency", ROTATION_TEST_TAG)
{
    vec3 vector = vec3(3.4, -2.23, 7.1);

    quaternion q = quaternion::eulerAngles(23, 45, 76);
    rot3x3 mat = rot3x3::eulerAngles(23, 45, 76);

    vec3 qResult = math::rotate(q, vector);
    vec3 matResult = mat * vector;

    CHECK(math::approx(qResult, matResult));
}

TEST_CASE("Quaternion normalize", ROTATION_TEST_TAG)
{
    quaternion q = quaternion(1, 2, 3, 4);
    q = normalize(q);
    float sqrMag = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    CHECK(math::approx(sqrMag, 1));
}
