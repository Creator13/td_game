#include <catch2/catch_all.hpp>

#include "math/mat4.h"

using math::mat4;
using math::vec4;

#define MATRIX_TEST_TAG "[math][matrix]"

namespace math
{
    std::ostream& operator<<(std::ostream& os, const mat4& m)
    {
        return os << "[ " <<
               m[0] << " " << m[4] << " " << m[8] << " " << m[12] << "; " <<
               m[1] << " " << m[5] << " " << m[9] << " " << m[13] << "; " <<
               m[2] << " " << m[6] << " " << m[10] << " " << m[14] << "; " <<
               m[3] << " " << m[7] << " " << m[11] << " " << m[15] << " ]";
    }
}

TEST_CASE("mat4 construction", MATRIX_TEST_TAG)
{
    SECTION("Parameter constructor")
    {
        mat4 mat = mat4(
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15);

        float expectedData[16] = {
            0, 4, 8, 12,
            1, 5, 9, 13,
            2, 6, 10, 14,
            3, 7, 11, 15
        };

        for (int i = 0; i < 16; i++)
        {
            CHECK(mat.m[i] == expectedData[i]);
        }
    }

    SECTION("Zero constructor")
    {
        mat4 mat = mat4();

        for (int i = 0; i < 16; i++)
        {
            CHECK(mat[i] == 0);
        }
    }
}

TEST_CASE("mat4 constants", MATRIX_TEST_TAG)
{
    SECTION("Zero matrix")
    {
        mat4 mat = mat4::zero;
        for (int i = 0; i < 16; i++)
        {
            CHECK(mat[i] == 0);
        }
    }

    SECTION("Identity matrix")
    {
        mat4 mat = mat4::identity;
        REQUIRE(mat == mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1));
    }
}

TEST_CASE("mat4 equality", MATRIX_TEST_TAG)
{
    mat4 a = mat4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);

    // Identical matrix
    mat4 b = mat4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);

    // Different values
    mat4 c = mat4(
        15, 14, 13, 12,
        11, 10, 9, 8,
        7, 6, 5, 4,
        3, 2, 1, 0);

    SECTION("operator==")
    {
        CHECK(a.operator==(a)); // Object equal to itself
        CHECK(a.operator==(b)); // Matrix equal to identical matrix
        CHECK_FALSE(a.operator==(c)); // Matrix not equal to different matrix
    }

    SECTION("operator!=")
    {
        CHECK(a.operator!=(c));
        CHECK_FALSE(a.operator!=(a));
        CHECK_FALSE(a.operator!=(b));
    }
}

TEST_CASE("mat4 getters", MATRIX_TEST_TAG)
{
    mat4 mat = mat4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);

    SECTION("Get row")
    {
        CHECK(mat.getCol(0) == vec4(0,4, 8,12));
        CHECK(mat.getCol(1) == vec4(1,5, 9,13));
        CHECK(mat.getCol(2) == vec4(2,6, 10,14));
        CHECK(mat.getCol(3) == vec4(3,7, 11,15));
    }

    SECTION("Get column")
    {
        CHECK(mat.getRow(0) == vec4(0, 1, 2, 3));
        CHECK(mat.getRow(1) == vec4(4, 5, 6, 7));
        CHECK(mat.getRow(2) == vec4(8, 9, 10, 11));
        CHECK(mat.getRow(3) == vec4(12, 13, 14, 15));
    }

    SECTION("Get element")
    {
        CHECK(mat.get(0, 0) == 0);
        CHECK(mat.get(0, 1) == 1);
        CHECK(mat.get(0, 2) == 2);
        CHECK(mat.get(0, 3) == 3);

        CHECK(mat.get(1, 0) == 4);
        CHECK(mat.get(1, 1) == 5);
        CHECK(mat.get(1, 2) == 6);
        CHECK(mat.get(1, 3) == 7);

        CHECK(mat.get(2, 0) == 8);
        CHECK(mat.get(2, 1) == 9);
        CHECK(mat.get(2, 2) == 10);
        CHECK(mat.get(2, 3) == 11);

        CHECK(mat.get(3, 0) == 12);
        CHECK(mat.get(3, 1) == 13);
        CHECK(mat.get(3, 2) == 14);
        CHECK(mat.get(3, 3) == 15);
    }
}

TEST_CASE("mat4 transpose", MATRIX_TEST_TAG)
{
    SECTION("Symmetric matrix is transpose of itself")
    {
        REQUIRE(mat4::zero.getTranspose() == mat4::zero);
        REQUIRE(mat4::identity.getTranspose() == mat4::identity);

        mat4 mat = mat4(
            0, 1, 2, 3,
            1, 1, -1, -1,
            2, -1, 2, 0,
            3, -1, 0, 2);
        REQUIRE(mat.getTranspose() == mat);
    }

    mat4 mat = mat4(
        4, 5, 4, 5,
        1, 2, 3, 4,
        -1, -2, -3, -4,
        9, 8, 9, 8);

    SECTION("Arbitrary matrix")
    {
        mat4 expected = mat4(
            4, 1, -1, 9,
            5, 2, -2, 8,
            4, 3, -3, 9,
            5, 4, -4, 8
        );
        mat4 result = mat.getTranspose();
        REQUIRE(result == expected);
    }

    SECTION("Double transpose")
    {
        REQUIRE(mat.getTranspose().getTranspose() == mat);
    }
}

TEST_CASE("mat4 identity", MATRIX_TEST_TAG)
{
    SECTION("Identity matrix is identity matrix")
    {
        REQUIRE(mat4::identity.isIdentity());
    }

    SECTION("Non-identity matrix")
    {
        mat4 mat = mat4(
            4, 5, 4, 5,
            1, 2, 3, 4,
            -1, -2, -3, -4,
            9, 8, 9, 8);
        REQUIRE_FALSE(mat.isIdentity());
        REQUIRE_FALSE(mat4::zero.isIdentity());
    }

    SECTION("Tolerance")
    {
        mat4 bad_tolerance = mat4(
            1.1, 0, 0, 0,
            0, 1.1, 0, 0,
            0, 0, 0.9, 0,
            0, 0, 0, 1);
        REQUIRE_FALSE(bad_tolerance.isIdentity()); // Falls far outside the default tolerance
        REQUIRE(bad_tolerance.isIdentity(.11f)); // Should be accepted with a higher tolerance

        mat4 right_tolerance = mat4(
            1.0000001, -.0000001, 0, 0.0000001,
            0, 1, 0, 0,
            0.0000001, 0, .9999999, 0,
            0, 0.00000099, 0, .99999998);
        REQUIRE(right_tolerance.isIdentity());
    }
}

using Catch::Matchers::WithinAbs;
constexpr float EPS = math::EPSILON;

TEST_CASE("mat4 determinant", MATRIX_TEST_TAG)
{
    SECTION("Zero and identity")
    {
        REQUIRE_THAT(mat4::identity.getDeterminant(), WithinAbs(1.0f, EPS));
        REQUIRE_THAT(mat4::zero.getDeterminant(), WithinAbs(0.0f, EPS));
    }

    SECTION("General")
    {
        using pair = std::pair<mat4, float>;
        auto [mat, expected] = GENERATE(
            pair(mat4(
                2, 0, 0, 0,
                0, 3, 0, 0,
                0, 0, 4, 0,
                0, 0, 0, 5), 120.f),
            pair(mat4(
                1, 2, 3, 4,
                5, 3, 2, 1,
                2, 1, 5, 3,
                4, 1, 2, 3), -126.f),
            pair(mat4(
                1, 2, 3, 4,
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12), 0.f),
            pair(mat4(
                2, 0, 0, -5.0f,
                0, 1, 0, 10.f,
                0, 0, 3, -20.f,
                0, 0, 0, 1), 6.f)
        );

        CHECK_THAT(mat.getDeterminant(), WithinAbs(expected, EPS));
    }
}

TEST_CASE("mat4 inverse", MATRIX_TEST_TAG)
{
    SECTION("Identity")
    {
        REQUIRE(math::approx(mat4::identity, mat4::identity.getInverse()));
    }

    SECTION("Translation matrix")
    {
        // The inverse should be a translation in the opposite direction.
        mat4 translation = mat4(
            1, 0, 0, 5.0f,
            0, 1, 0, -10.f,
            0, 0, 1, 20.f,
            0, 0, 0, 1);

        mat4 expectedInvTranslation = mat4(
            1, 0, 0, -5.0f,
            0, 1, 0, 10.f,
            0, 0, 1, -20.f,
            0, 0, 0, 1);

        mat4 invTranslation = translation.getInverse();
        REQUIRE(math::approx(invTranslation, expectedInvTranslation));
    }

    SECTION("M * M-1 = I")
    {
        mat4 general(
            2, 0, 1, 0,
            0, 1, 0, 2,
            1, 0, 2, 0,
            0, 3, 0, 1
        );
        REQUIRE_THAT(general.getDeterminant(), !WithinAbs(0.0f, EPS));

        mat4 invGeneral = general.getInverse();
        mat4 result = general * invGeneral;

        REQUIRE(math::approx(result, mat4::identity));
    }

    SECTION("Singular matrix failure")
    {
        mat4 singular(
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 10, 11, 12,
            9, 10, 11, 12
        );
        REQUIRE_THAT(singular.getDeterminant(), WithinAbs(0.0f, EPS));

        mat4 invSingular = singular.getInverse();
        REQUIRE(math::approx(invSingular, mat4::zero));
    }
}

TEST_CASE("mat4 multiplication", MATRIX_TEST_TAG)
{
    mat4 a = mat4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);
    mat4 b = mat4(
        3, 4, 5, 6,
        1, 2, 3, 4,
        5, 6, 7, 8,
        0, 2, 4, 6);

    SECTION("Matrix * matrix")
    {
        mat4 result = mat4(
            11, 20, 29, 38,
            47, 76, 105, 134,
            83, 132, 181, 230,
            119, 188, 257, 326);
        CHECK(math::approx(a * b, result));
    }

    SECTION("Non-commutative")
    {
        CHECK_FALSE(math::approx(a * b, b * a));
    }

    SECTION("Associative")
    {
        mat4 c = mat4(
            1, 2, 1, 2,
            3, 4, 3, 4,
            5, 6, 5, 6,
            1, 4, 2, 8);

        CHECK(math::approx((a * b) * c, a * (b * c)));
    }

    SECTION("Identity")
    {
        CHECK(math::approx(mat4::identity * a, a));
        CHECK(math::approx(a * mat4::identity, a));
    }

    SECTION("Zero")
    {
        CHECK(math::approx(a * mat4::zero, mat4::zero));
        CHECK(math::approx(mat4::zero * a, mat4::zero));
        CHECK(math::approx(mat4::zero * mat4::identity, mat4::zero));
    }

    SECTION("Transpose")
    {
        mat4 transposeA = a.getTranspose();
        mat4 transposeB = b.getTranspose();

        REQUIRE(math::approx(transposeA * transposeB, (b * a).getTranspose()));
    }

    SECTION("Determinant")
    {
        float detA = a.getDeterminant();
        float detB = b.getDeterminant();
        CHECK_THAT((a * b).getDeterminant(), WithinAbs(detA * detB, EPS));
    }

    SECTION("Invertible matrices")
    {
        mat4 mat = mat4(
            2, 1, 0, 3,
            0, 1, 4, -1,
            0, 0, 3, 2,
            0, 0, 0, 1);
        mat4 mat2 = mat4(
            3, 0, 2, -1,
            1, 2, 0, -2,
            4, 0, 6, -3,
            0, 0, 0, 2);
        CHECK((mat * mat.getInverse()).isIdentity());
        CHECK(math::approx((mat * mat2).getInverse(), mat2.getInverse() * mat.getInverse()));
    }
}

TEST_CASE("Matrix-vector multiplication", MATRIX_TEST_TAG)
{
    SECTION("General affine")
    {
        mat4 mat = mat4(
            2, 0, 0, 5,
            0, 1, 0, 1.52f,
            0, 0, 3, 0.2f,
            0, 0, 0, 1);
        vec4 vec = vec4(1, 2, 4, 1);
        vec4 expected = vec4(7, 3.52, 12.2, 1);
        REQUIRE(math::approx(mat * vec, expected));
    }

    SECTION("Arbitrary")
    {
        mat4 mat = mat4(
            1, 2, 3, -4,
            5, -6, 5, 6,
            7, 8, 9.1, -10,
            3, -4, 3, 4);
        vec4 vec = vec4(-1.5, 2, 1.2, 2.5);
        vec4 expected = vec4(-3.9, 1.5, -8.58, 1.1);
        REQUIRE(math::approx(mat * vec, expected));
    }
}

TEST_CASE("Transformations matrices", MATRIX_TEST_TAG)
{
    SECTION("Translation")
    {
        mat4 t = mat4::makeTranslate(2, .5f, 18.23f);
        SECTION("Point")
        {
            vec4 point = vec4(1, 1, 1, 1);
            vec4 result = t * point;
            // Multiplying a vector by a transformation matrix yields a translated point
            CHECK(math::approx(result, vec4(1 + 2, 1 + .5f, 1 + 18.23f, 1)));
            // Multiplying with the inverse yields the original vector
            CHECK(math::approx(t.getInverse() * result, point));
        }
        SECTION("Direction")
        {
            // Multiplying a vec4 which represents a direction (ie vec4.w = 0) yields the original vector
            vec4 dir = vec4(1, 1, 1, 0);
            vec4 result = t * dir;
            CHECK(math::approx(result, dir));
        }
    }

    SECTION("Scaling")
    {
        mat4 s = mat4::makeScale(4, 2, 3);
        vec4 vec = vec4(1, 2, 3, 1);
        vec4 result = s * vec;
        CHECK(math::approx(result, vec4(1 * 4, 2 * 2, 3 * 3, 1)));
        CHECK(math::approx(s.getInverse() * result, vec));
    }

    SECTION("Overloads")
    {
        // The overloads that take a vec3 and separate coords produce the same matrices
        float x = 2, y = 4, z = 6;
        math::vec3 vec = math::vec3(x, y, z);
        CHECK(mat4::makeTranslate(x, y, z) == mat4::makeTranslate(vec));
        CHECK(mat4::makeScale(x, y, z) == mat4::makeScale(vec));
    }
}
