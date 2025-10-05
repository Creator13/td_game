#include <catch2/catch_all.hpp>

#include "math/mat4.h"
#include "math/func.h"
#include "math/rotation.h"
#include "math/vec.h"

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
        rot3x3 invRot = transpose(rot);
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

TEMPLATE_TEST_CASE("Euler angles", ROTATION_TEST_TAG, quaternion, rot3x3)
{
    using RotationType = TestType;

    SECTION("Canonical angles")
    {
        vec3 angles = GENERATE(
            vec3(-30, 20, 56),
            vec3(34.4f, -89, 32),
            vec3(0, 18, -122),
            vec3(-179.4f, 0, 179.5f),
            vec3(0, 0, 0),
            vec3(118, 20, 0),
            vec3(90, 0, 0),
            vec3 (0, 45, 0),
            vec3(0, 0, -90)
        );

        RotationType rot = RotationType::eulerAngles(angles);
        vec3 result = math::toEuler(rot);
        // Requires a fairly large epsilon, because the numbers go through quite a few transformations.
        // Unfortunately this is the nature of rotations.
        CHECK(math::approx(angles, result, 2e-4f));
    }

    SECTION("Gimbal lock")
    {
        vec3 angles = GENERATE(
            vec3(45, 90, 0),
            vec3(-21.15, 90, 143),
            vec3(60, 90, -143),
            vec3(60, -90, 20),
            vec3(-60, -90, 122),
            vec3(0, -90, 0)
        );

        RotationType rot = RotationType::eulerAngles(angles);
        vec3 result = math::toEuler(rot);

        CHECK(math::approx(result.y, angles.y)); // Returned y angle is identical to the input y angle.
        CHECK(result.z == 0); // Z is expected to be exactly zero as it cannot be recovered in gimbal lock.

        // Compare sin and cos of combined x and z input angles to be equal to the sin and cos of the output x angle.
        // This ensures the resulting angles are equivalent even if they do not match numerically.
        float expectedCombined;
        if (angles.y > 0)
        {
            expectedCombined = angles.x - angles.z;
        }
        else
        {
            expectedCombined = angles.x + angles.z;
        }
        expectedCombined *= math::DEG2RAD;
        float resultX = result.x * math::DEG2RAD;
        CHECK(math::approx(math::cos(expectedCombined), math::cos(resultX)));
        CHECK(math::approx(math::sin(expectedCombined), math::sin(resultX)));
    }

    SECTION("Boundary angle checks")
    {
        vec3 angles = GENERATE(
            vec3(180, 0, 0),
            vec3(-180, 0, 0),
            vec3(0, 180, 0),
            vec3(0, -180, 0),
            vec3(0, 0, 180),
            vec3(0, 0, -180)
        );
        RotationType rot = RotationType::eulerAngles(angles);
        vec3 result = math::toEuler(rot);

        rot3x3 originalAnglesMatrix = rot3x3::eulerAngles(angles);
        rot3x3 convertedAnglesMatrix = rot3x3::eulerAngles(result);
        CHECK(math::approx(originalAnglesMatrix, convertedAnglesMatrix));
    }

    SECTION("Composition")
    {
        // Verify that euler angle rotation matches rot_z * rot_y * rot_x where rot_# is a rotation about the corresponding cardinal axis
        vec3 angles = GENERATE(
            vec3(-30, 20, 56),
            vec3(34.4f, -89, 32),
            vec3 (0, 45, 0),
            vec3(0, 0, -90)
        );

        RotationType rotExpected = RotationType::eulerAngles(angles);

        RotationType rotX = RotationType::angleAxis(angles.x, vec3(1, 0, 0));
        RotationType rotY = RotationType::angleAxis(angles.y, vec3(0, 1, 0));
        RotationType rotZ = RotationType::angleAxis(angles.z, vec3(0, 0, 1));

        RotationType rotResult = rotZ * rotY * rotX;
        CHECK(math::approx(rotResult, rotExpected));
    }
}

TEMPLATE_TEST_CASE("Angle-axis", ROTATION_TEST_TAG, quaternion, rot3x3)
{
    using RotationType = TestType;

    SECTION("Zero angle")
    {
        // Rotation of 0 returns an identity rotation
        RotationType rot = RotationType::angleAxis(0, vec3(0, 0, 1));
        vec3 input = vec3(50, 1, -3.3);
        vec3 result = math::rotate(rot, input);
        CHECK(math::approx(result, input));
    }

    SECTION("Zero axis")
    {
        // Rotation around an undefined axis is not legal; this returns an identity rotation
        RotationType rot = RotationType::angleAxis(180, vec3::zero);
        vec3 input = vec3(1, 0, 0);
        vec3 result = math::rotate(rot, input);
        CHECK(math::approx(result, input));
    }

    SECTION("Non-normalized axis")
    {
        // Using a non-normalized axis should result in that axis being normalized and result in a valid, normalized rotation
        vec3 axis = GENERATE(
            vec3(0, 0, 2.34f),
            vec3(0, 0, 0.103f)
        );

        RotationType rot = RotationType::angleAxis(90, axis);

        vec3 input = vec3(1, 0, 0);
        vec3 expected = vec3(0, 1, 0);
        vec3 result = math::rotate(rot, input);
        CHECK(math::approx(result, expected));
    }
}

TEMPLATE_TEST_CASE("LookRotation", ROTATION_TEST_TAG, quaternion, rot3x3)
{
    using RotationType = TestType;

    SECTION("Cardinal axis")
    {
        RotationType rot = RotationType::lookRotation(vec3(1, 0, 0), vec3(0, 0, 1));
        vec3 in = vec3(1, 0, 0);
        vec3 result = math::rotate(rot, in);
        CHECK(math::approx(result, vec3(0, -1, 0)));
    }
}

TEST_CASE("Rotation matrix orthonormalness", ROTATION_TEST_TAG)
{
    vec3 angles = GENERATE(
        vec3(34, -58, 122),
        vec3(400, 23.32322, -400),
        vec3(-0, -10000, 2.22),
        vec3(0, 0, 0)
    );
    rot3x3 rot = rot3x3::eulerAngles(angles);

    SECTION("Basis vectors are orthogonal")
    {
        CHECK(math::approx(dot(rot.xBasis, rot.yBasis), 0.f));
        CHECK(math::approx(dot(rot.xBasis, rot.zBasis), 0.f));
        CHECK(math::approx(dot(rot.yBasis, rot.zBasis), 0.f));
    }

    SECTION("Basis vectors are unit length")
    {
        CHECK(math::approx(rot.xBasis.length(), 1.f));
        CHECK(math::approx(rot.yBasis.length(), 1.f));
        CHECK(math::approx(rot.zBasis.length(), 1.f));
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
    vec3 angles = GENERATE(
        vec3(23, 45, 76),
        vec3(-25.3, 18, 190),
        vec3(0, 10, 890),
        vec3(500,500 , 0),
        vec3(0, 500, 0),
        vec3(0, -500,0),
        vec3(0, 0, 190),
        vec3(40, 190, -270),
        vec3(0, 10, 190)
    );

    quaternion q = quaternion::eulerAngles(angles);
    rot3x3 mat = rot3x3::eulerAngles(angles);

    vec3 vector = vec3(3.4, -2.23, 7.1);
    SECTION("Rotation application")
    {
        vec3 qResult = math::rotate(q, vector);
        vec3 matResult = mat * vector;

        CHECK(math::approx(qResult, matResult));
    }

    SECTION("Conversion")
    {
        SECTION("Matrix to quaternion")
        {
            quaternion qFromMat = quaternion::fromRot3x3(mat);

            vec3 result_originalQuaternion = math::rotate(q, vector);
            vec3 result_convertedQuaterion = math::rotate(qFromMat, vector);

            CHECK(math::approx(math::abs(dot(q, qFromMat)), 1));
            CHECK(math::approx(result_convertedQuaterion, result_originalQuaternion));
        }
        SECTION("Quaternion to matrix")
        {
            rot3x3 matFromQ = rot3x3::fromQuaternion(q);

            vec3 result_originalMatrix = mat * vector;
            vec3 result_convertedMatrix = matFromQ * vector;

            CHECK(math::approx(mat, matFromQ));
            CHECK(math::approx(result_convertedMatrix, result_originalMatrix));
        }
    }
}

TEST_CASE("Slerp", ROTATION_TEST_TAG)
{
    SECTION("Edge cases")
    {
        quaternion q0 = quaternion::eulerAngles(78, 1, 23.4);
        quaternion q1 = quaternion::eulerAngles(-20.2, 98, 30.3);

        CHECK(math::slerp(q0, q1, 0) == q0);
        CHECK(math::slerp(q0, q1, 1) == q1);

        CHECK(math::slerp(q0, q0, 0) == q0);
        CHECK(math::approx(math::slerp(q0, q0, .234), q0));
        CHECK(math::slerp(q0, q0, 1) == q0);
    }

    SECTION("Functionality")
    {
        quaternion q0 = quaternion::identity;
        quaternion q1 = quaternion::angleAxis(90, vec3(1, 0, 0));

        quaternion expected = quaternion::angleAxis(45, vec3(1, 0, 0));
        CHECK(math::approx(math::slerp(q0, q1, .5f), expected));
    }
}

TEST_CASE("Quaternion normalize", ROTATION_TEST_TAG)
{
    quaternion q = quaternion(1, 2, 3, 4);
    q = normalize(q);
    float sqrMag = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    CHECK(math::approx(sqrMag, 1));
}
