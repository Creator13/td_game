#include <catch2/catch_all.hpp>

#include "math/mat4x4.h"

using math::mat4x4;
using math::vec4;

#define MATRIX_TEST_TAG "[math][matrix]"

namespace math {
    std::ostream& operator<<(std::ostream& os, const mat4x4& m) {
        return os << "[ " <<
               m[0] << " " << m[4] << " " << m[8] << " " << m[12] << "; " <<
               m[1] << " " << m[5] << " " << m[9] << " " << m[13] << "; " <<
               m[2] << " " << m[6] << " " << m[10] << " " << m[14] << "; " <<
               m[3] << " " << m[7] << " " << m[11] << " " << m[15] << " ]";
    }
}

TEST_CASE("mat4x4 construction", MATRIX_TEST_TAG) {
    SECTION("Parameter constructor") {
        mat4x4 mat = mat4x4(
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

        for (int i = 0; i < 16; i++) {
            CHECK(mat.m[i] == expectedData[i]);
        }
    }

    SECTION("Zero constructor") {
        mat4x4 mat = mat4x4();

        for (int i = 0; i < 16; i++) {
            CHECK(mat[i] == 0);
        }
    }
}

TEST_CASE("mat4x4 constants", MATRIX_TEST_TAG) {
    SECTION("Zero matrix") {
        mat4x4 mat = mat4x4::zero;
        for (int i = 0; i < 16; i++) {
            CHECK(mat[i] == 0);
        }
    }

    SECTION("Identity matrix") {
        mat4x4 mat = mat4x4::identity;
        REQUIRE(mat == mat4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1));
    }
}

TEST_CASE("mat4x4 equality", MATRIX_TEST_TAG) {
    mat4x4 a = mat4x4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);

    // Identical matrix
    mat4x4 b = mat4x4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);

    // Different values
    mat4x4 c = mat4x4(
        15, 14, 13, 12,
        11, 10, 9, 8,
        7, 6, 5, 4,
        3, 2, 1, 0);

    SECTION("operator==") {
        CHECK(a.operator==(a)); // Object equal to itself
        CHECK(a.operator==(b)); // Matrix equal to identical matrix
        CHECK_FALSE(a.operator==(c)); // Matrix not equal to different matrix
    }

    SECTION("operator!=") {
        CHECK(a.operator!=(c));
        CHECK_FALSE(a.operator!=(a));
        CHECK_FALSE(a.operator!=(b));
    }
}

TEST_CASE("mat4x4 getters", MATRIX_TEST_TAG) {
    mat4x4 mat = mat4x4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);

    SECTION("Get row") {
        CHECK(mat.getCol(0) == vec4(0,4, 8,12));
        CHECK(mat.getCol(1) == vec4(1,5, 9,13));
        CHECK(mat.getCol(2) == vec4(2,6, 10,14));
        CHECK(mat.getCol(3) == vec4(3,7, 11,15));
    }

    SECTION("Get column") {
        CHECK(mat.getRow(0) == vec4(0, 1, 2, 3));
        CHECK(mat.getRow(1) == vec4(4, 5, 6, 7));
        CHECK(mat.getRow(2) == vec4(8, 9, 10, 11));
        CHECK(mat.getRow(3) == vec4(12, 13, 14, 15));
    }

    SECTION("Get element") {
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

TEST_CASE("mat4x4 transpose", MATRIX_TEST_TAG) {
    SECTION("Symmetric matrix is transpose of itself") {
        REQUIRE(mat4x4::zero.getTranspose() == mat4x4::zero);
        REQUIRE(mat4x4::identity.getTranspose() == mat4x4::identity);

        mat4x4 mat = mat4x4(
            0, 1, 2, 3,
            1, 1, -1, -1,
            2, -1, 2, 0,
            3, -1, 0, 2);
        REQUIRE(mat.getTranspose() == mat);
    }

    mat4x4 mat = mat4x4(
        4, 5, 4, 5,
        1, 2, 3, 4,
        -1, -2, -3, -4,
        9, 8, 9, 8);

    SECTION("Arbitrary matrix") {
        mat4x4 expected = mat4x4(
            4, 1, -1, 9,
            5, 2, -2, 8,
            4, 3, -3, 9,
            5, 4, -4, 8
        );
        mat4x4 result = mat.getTranspose();
        REQUIRE(result == expected);
    }

    SECTION("Double transpose") {
        REQUIRE(mat.getTranspose().getTranspose() == mat);
    }
}

TEST_CASE("mat4x4 identity", MATRIX_TEST_TAG) {
    SECTION("Identity matrix is identity matrix") {
        REQUIRE(mat4x4::identity.isIdentity());
    }

    SECTION("Non-identity matrix") {
        mat4x4 mat = mat4x4(
            4, 5, 4, 5,
            1, 2, 3, 4,
            -1, -2, -3, -4,
            9, 8, 9, 8);
        REQUIRE_FALSE(mat.isIdentity());
        REQUIRE_FALSE(mat4x4::zero.isIdentity());
    }

    SECTION("Tolerance") {
        mat4x4 bad_tolerance = mat4x4(
            1.1, 0, 0, 0,
            0, 1.1, 0, 0,
            0, 0, 0.9, 0,
            0, 0, 0, 1);
        REQUIRE_FALSE(bad_tolerance.isIdentity()); // Falls far outside the default tolerance
        REQUIRE(bad_tolerance.isIdentity(.11f)); // Should be accepted with a higher tolerance

        mat4x4 right_tolerance = mat4x4(
            1.0000001, -.0000001, 0, 0.0000001,
            0, 1, 0, 0,
            0.0000001, 0, .9999999, 0,
            0, 0.00000099, 0, .99999998);
        REQUIRE(right_tolerance.isIdentity());
    }
}

using Catch::Matchers::WithinAbs;
constexpr float EPS = math::EPSILON;

TEST_CASE("mat4x4 determinant", MATRIX_TEST_TAG) {
    SECTION("Zero and identity") {
        REQUIRE_THAT(mat4x4::identity.getDeterminant(), WithinAbs(1.0f, EPS));
        REQUIRE_THAT(mat4x4::zero.getDeterminant(), WithinAbs(0.0f, EPS));
    }

    SECTION("General") {
        using pair = std::pair<mat4x4, float>;
        auto [mat, expected] = GENERATE(
            pair(mat4x4(
                2, 0, 0, 0,
                0, 3, 0, 0,
                0, 0, 4, 0,
                0, 0, 0, 5), 120.f),
            pair(mat4x4(
                1, 2, 3, 4,
                5, 3, 2, 1,
                2, 1, 5, 3,
                4, 1, 2, 3), -126.f),
            pair(mat4x4(
                1, 2, 3, 4,
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12), 0.f),
            pair(mat4x4(
                2, 0, 0, -5.0f,
                0, 1, 0, 10.f,
                0, 0, 3, -20.f,
                0, 0, 0, 1), 6.f)
        );

        CHECK_THAT(mat.getDeterminant(), WithinAbs(expected, EPS));
    }
}

TEST_CASE("mat4x4 inverse", MATRIX_TEST_TAG) {
    SECTION("Identity") {
        REQUIRE(math::approx(mat4x4::identity, mat4x4::identity.getInverse()));
    }

    SECTION("Translation matrix") {
        // The inverse should be a translation in the opposite direction.
        mat4x4 translation = mat4x4(
            1, 0, 0, 5.0f,
            0, 1, 0, -10.f,
            0, 0, 1, 20.f,
            0, 0, 0, 1);

        mat4x4 expectedInvTranslation = mat4x4(
            1, 0, 0, -5.0f,
            0, 1, 0, 10.f,
            0, 0, 1, -20.f,
            0, 0, 0, 1);

        mat4x4 invTranslation = translation.getInverse();
        REQUIRE(math::approx(invTranslation, expectedInvTranslation));
    }

    SECTION("M * M-1 = M1") {
        mat4x4 general(
            2, 0, 1, 0,
            0, 1, 0, 2,
            1, 0, 2, 0,
            0, 3, 0, 1
        );
        REQUIRE_THAT(general.getDeterminant(), !WithinAbs(0.0f, EPS));

        mat4x4 invGeneral = general.getInverse();
        mat4x4 result = general * invGeneral;

        REQUIRE(math::approx(result, mat4x4::identity));
    }

    SECTION("Singular matrix failure") {
        mat4x4 singular(
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 10, 11, 12,
            9, 10, 11, 12
        );
        REQUIRE_THAT(singular.getDeterminant(), WithinAbs(0.0f, EPS));

        mat4x4 invSingular = singular.getInverse();
        REQUIRE(math::approx(invSingular, mat4x4::zero));
    }
}

TEST_CASE("mat4x4 multiplication", MATRIX_TEST_TAG) {
    mat4x4 a = mat4x4(
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15);
    mat4x4 b = mat4x4(
        3, 4, 5, 6,
        1, 2, 3, 4,
        5, 6, 7, 8,
        0, 2, 4, 6);

    SECTION("Matrix * matrix") {
        mat4x4 result = mat4x4(
            11, 20, 29, 38,
            47, 76, 105, 134,
            83, 132, 181, 230,
            119, 188, 257, 326);
        REQUIRE(math::approx(a * b, result));
    }

    SECTION("Non-commutative") {
        REQUIRE_FALSE(math::approx(a * b, b * a));
    }

    SECTION("Associative") {
        mat4x4 c = mat4x4(
            1, 2, 1, 2,
            3, 4, 3, 4,
            5, 6, 5, 6,
            1, 4, 2, 8);

        REQUIRE(math::approx((a * b) * c, a * (b * c)));
    }

    SECTION("Identity") {
        REQUIRE(math::approx(mat4x4::identity * a, a));
        REQUIRE(math::approx(a * mat4x4::identity, a));
    }

    SECTION("Zero") {
        REQUIRE(math::approx(a * mat4x4::zero, mat4x4::zero));
        REQUIRE(math::approx(mat4x4::zero * a, mat4x4::zero));
        REQUIRE(math::approx(mat4x4::zero * mat4x4::identity, mat4x4::zero));
    }

    SECTION("Transpose") {
        mat4x4 transposeA = a.getTranspose();
        mat4x4 transposeB = b.getTranspose();

        REQUIRE(math::approx(transposeA * transposeB, (b * a).getTranspose()));
    }

    SECTION("Determinant") {
        float detA = a.getDeterminant();
        float detB = b.getDeterminant();
        REQUIRE_THAT((a * b).getDeterminant(), WithinAbs(detA * detB, EPS));
    }

    SECTION("Invertible matrices") {
        mat4x4 mat = mat4x4(
            2, 1, 0, 3,
            0, 1, 4, -1,
            0, 0, 3, 2,
            0, 0, 0, 1);
        mat4x4 mat2 = mat4x4(
            3, 0, 2, -1,
            1, 2, 0, -2,
            4, 0, 6, -3,
            0, 0, 0, 2);
        REQUIRE((mat * mat.getInverse()).isIdentity());
        REQUIRE(math::approx((mat * mat2).getInverse(), mat2.getInverse() * mat.getInverse()));
    }
}

TEST_CASE("Matrix-vector multiplication", MATRIX_TEST_TAG) {
    SECTION("General affine") {
        mat4x4 mat = mat4x4(
            2, 0, 0, 5,
            0, 1, 0, 1.52f,
            0, 0, 3, 0.2f,
            0, 0, 0, 1);
        vec4 vec = vec4(1, 2, 4, 1);
        vec4 expected = vec4(7, 3.52, 12.2, 1);
        REQUIRE(math::approx(mat * vec, expected));
    }

    SECTION("Arbitrary") {
        mat4x4 mat = mat4x4(
            1, 2, 3, -4,
            5, -6, 5, 6,
            7, 8, 9.1, -10,
            3, -4, 3, 4);
        vec4 vec = vec4(-1.5, 2, 1.2, 2.5);
        vec4 expected = vec4(-3.9, 1.5, -8.58, 1.1);
        REQUIRE(math::approx(mat * vec, expected));
    }
}
