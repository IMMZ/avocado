#include "../src/math/matrix.hpp"
#include "../src/math/vecn.hpp"

#include <catch_amalgamated.hpp>

using namespace avocado::math;

TEST_CASE("Matrix properties") {
    constexpr Mat4x4 mat ({{
            {1.f, 8.f, 0.f, 0.f},
            {0.f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 1.f, 0.f},
            {0.f, 0.f, 0.f, 1.f}
    }});

    constexpr Mat4x4 unitMatrix ({{
            {1.f, 0.f, 0.f, 0.f},
            {0.f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 1.f, 0.f},
            {0.f, 0.f, 0.f, 1.f}
    }});

    constexpr Mat4x4 nullMatrix ({{
            {0.f, 0.f, 0.f, 0.f},
            {0.f, 0.f, 0.f, 0.f},
            {0.f, 0.f, 0.f, 0.f},
            {0.f, 0.f, 0.f, 0.f}
    }});

    constexpr Mat4x4 defaultMatrix{{}};

    SECTION("Square test") {
        REQUIRE(mat.isSquare());
    }

    SECTION("Unit matrix") {
        REQUIRE_FALSE(mat.isIdentity());
        REQUIRE_FALSE(nullMatrix.isIdentity());
        REQUIRE_FALSE(defaultMatrix.isIdentity());
        REQUIRE(unitMatrix.isIdentity());
    }

    SECTION("Null matrix") {
        REQUIRE(nullMatrix.isNull());
        REQUIRE(defaultMatrix.isNull());
        REQUIRE_FALSE(mat.isNull());
        REQUIRE_FALSE(unitMatrix.isNull());
    }

    SECTION("Equality") {
        Mat4x4 unitCopyMatrix = unitMatrix;
        REQUIRE(unitCopyMatrix == unitMatrix);
        REQUIRE(mat == mat);
        REQUIRE_FALSE(mat == unitMatrix);
        REQUIRE_FALSE(unitMatrix == nullMatrix);
    }
}

TEST_CASE("Matrix operations") {
    constexpr Mat4x4 unitMatrix ({{
            {1.f, 0.f, 0.f, 0.f},
            {0.f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 1.f, 0.f},
            {0.f, 0.f, 0.f, 1.f}
    }});

    constexpr Mat4x4 nullMatrix ({{
            {0.f, 0.f, 0.f, 0.f},
            {0.f, 0.f, 0.f, 0.f},
            {0.f, 0.f, 0.f, 0.f},
            {0.f, 0.f, 0.f, 0.f}
    }});

    constexpr Mat4x4 matrix1 ({{
            {2.2f, 1.f, 10.f, 1.1f},
            {0.f, 0.f, 17.f, 4.5f},
            {0.f, 31.3f, 0.f, 5.5f},
            {11.f, 0.f, 0.f, 6.5f}
    }});

    constexpr Mat4x4 matrix2 ({{
            {4.f, 2.2f, 6.f, 0.f},
            {0.f, 0.f, 0.f, 0.f},
            {11.5f, 7.f, 3.3f, 4.f},
            {35.f, 14.2f, 8.1f, 0.f}
    }});

    SECTION("Sum") {
        constexpr Mat4x4 result ({{
            {6.2f, 3.2f, 16.f, 1.1f},
            {0.f, 0.f, 17.f, 4.5f},
            {11.5f, 38.3f, 3.3f, 9.5f},
            {46.f, 14.2f, 8.1f, 6.5f}
        }});
        REQUIRE((matrix1 + matrix2) == result);
        REQUIRE((matrix1 + nullMatrix) == matrix1);
    }

    SECTION("Subtraction") {
        constexpr Mat4x4 result ({{
            {-1.8f, -1.2f, 4.f, 1.1f},
            {0.f, 0.f, 17.f, 4.5f},
            {-11.5f, 24.3f, -3.3f, 1.5f},
            {-24.f, -14.2f, -8.1f, 6.5f}
        }});
        REQUIRE((matrix1 - matrix2) == result);
        REQUIRE((matrix1 - nullMatrix) == matrix1);
    }

    SECTION("Multiplication by other matrix") {
        constexpr Mat4x4 result ({{
            {162.3f, 90.46f, 55.11f, 40.f},
            {353.f, 182.9f, 92.55f, 68.f},
            {192.5f, 78.1f, 44.55f, 0.f},
            {271.5f, 116.5f, 118.65f, 0.f}
        }});
        REQUIRE((matrix1 * matrix2) == result);
        REQUIRE_FALSE((matrix1 * matrix2) == (matrix2 * matrix1));
        REQUIRE((matrix1 * nullMatrix) == nullMatrix);
        REQUIRE((matrix1 * unitMatrix) == matrix1);
    }

    SECTION("Multiplication by number") {
        constexpr Mat4x4 result ({{
            {4.4f, 2.f, 20.f, 2.2f},
            {0.f, 0.f, 34.f, 9.f},
            {0.f, 62.6f, 0.f, 11.f},
            {22.f, 0.f, 0.f, 13.f}
        }});
        REQUIRE((matrix1 * 0) == nullMatrix);
        REQUIRE((matrix1 * 1) == matrix1);
        REQUIRE((matrix1 * 2) == result);
        REQUIRE((2.f * matrix1) == result);
    }

    SECTION("Multiplication by vector") {
        constexpr vec4f result(120.f, 110.f, 304.f, 294.f);
        constexpr vec4f someVector(10.f, 11.f, 12.f, 13.f);
        constexpr Mat4x4 someMatrix({{
            {1.f, 2.f, 3.f, 4.f},
            {4.f, 3.f, 2.f, 1.f},
            {5.f, 6.f, 7.f, 8.f},
            {8.f, 7.f, 6.f, 5.f}
        }});
        REQUIRE((someMatrix * someVector) == result);
        REQUIRE((someVector * someMatrix) == result);
    }

    SECTION("Transposing") {
        constexpr Mat4x4 result ({{
            {2.2f, 0.f, 0.f, 11.f},
            {1.f, 0.f, 31.3f, 0.f},
            {10.f, 17.f, 0.f, 0.f},
            {1.1f, 4.5f, 5.5f, 6.5f}
        }});
        REQUIRE(matrix1.transpose() == result);
        REQUIRE(matrix1.transpose().transpose() == matrix1);
        REQUIRE((matrix1 + matrix2).transpose() == (matrix1.transpose() + matrix2.transpose()));
        REQUIRE((matrix1 * matrix2).transpose() == (matrix2.transpose() * matrix1.transpose()));
        REQUIRE((matrix1 * 5).transpose() == (matrix1.transpose() * 5));
    }
}

