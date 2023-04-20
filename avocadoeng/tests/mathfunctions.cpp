#include "../src/math/functions.hpp"
#include "../src/math/matrix.hpp"
#include "../src/math/quaternion.hpp"

#include <catch_amalgamated.hpp>

using namespace avocado::math;

TEST_CASE("General functions") {
    SECTION("Degrees and radian conversions") {
        constexpr float epsilonToCompare = 0.0001f;
        REQUIRE(avocado::core::areFloatsEq(toDegrees(1.4f), 80.2141f, epsilonToCompare));
        REQUIRE(avocado::core::areFloatsEq(toDegrees(2.22f), 127.1966f, epsilonToCompare));
        REQUIRE(avocado::core::areFloatsEq(toDegrees(0.f), 0.f));
        REQUIRE(avocado::core::areFloatsEq(toRadians(1.4f), 0.0244346f));
        REQUIRE(avocado::core::areFloatsEq(toRadians(2.22f), 0.03874631f));
        REQUIRE(avocado::core::areFloatsEq(toRadians(0.f), 0.f));
    }

    SECTION("Rotation matrix creation") {
        const Mat4x4 m1 = createRotationMatrix(45.f, vec3f{0.f, 0.f, 1.f});
        constexpr Mat4x4 expectedMatrix({{
            {0.7071f, -0.7071f, 0.f, 0.f},
            {0.7071f,  0.7071f, 0.f, 0.f},
            {0.f,      0.f,     1.f, 0.f},
            {0.f,      0.f,     0.f, 1.f}
        }});
        REQUIRE(m1 == expectedMatrix);
        
        const Mat4x4 m2 = createRotationMatrix(57.f, vec3f{1.f, 0.f, 1.f});
        constexpr Mat4x4 expectedMatrix2({{
            {0.7723195f, -0.5930296f,  0.2276805f, 0.f},
            {0.5930296f,  0.5446391f, -0.5930296f, 0.f},
            {0.2276805f,  0.5930296f,  0.7723195f, 0.f},
            {0.f,         0.f,         0.f,        1.f}
        }});
        REQUIRE(m2 == expectedMatrix2);

        const Mat4x4 m3 = createRotationMatrix(Quaternion{1.f, 2.f, 3.f, 4.f});
        constexpr Mat4x4 expectedMatrix3({{
            { 0.1333333f, -0.6666667f, 0.7333333f, 0.f},
            { 0.9333333f,  0.3333333f, 0.1333333f, 0.f},
            {-0.3333333f,  0.6666667f, 0.6666667f, 0.f},
            { 0.f,         0.f,        0.f,        1.f}
        }});
        REQUIRE(m3 == expectedMatrix3);
    }
}

