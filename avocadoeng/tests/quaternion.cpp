#include "../src/math/quaternion.hpp"

#include <catch_amalgamated.hpp>

using namespace avocado::math;

TEST_CASE("General operations") {
    constexpr Quaternion q1{3.f, 5.f, 2.f, 6.f};
    constexpr Quaternion q2{1.f, 2.f, 3.f, 4.f};
    constexpr Quaternion q2Reversed{4.f, 3.f, 2.f, 1.f};

    SECTION("Addition") {
        REQUIRE(
            Quaternion{2.f, 3.f, 4.f, 1.f} + Quaternion{3.f, 3.f, 3.f, 1.f} == Quaternion{5.f, 6.f, 7.f, 2.f});

        // a + b == b + a
        REQUIRE(
            Quaternion{2.f, 0.f, 4.f, -1.f} + Quaternion{0.f, -3.f, 3.f, 1.f} == Quaternion{2.f, -3.f, 7.f, 0.f});
        REQUIRE(
            Quaternion{0.f, -3.f, 3.f, 1.f} + Quaternion{2.f, 0.f, 4.f, -1.f} == Quaternion{2.f, -3.f, 7.f, 0.f});

        // a + 0 == a.
        REQUIRE(
            Quaternion{} + Quaternion{0.f, -3.f, 3.f, 1.f} == Quaternion{0.f, -3.f, 3.f, 1.f});

        //-a + a == 0
        REQUIRE(
            Quaternion{-1.f, -2.f, -3.f, -3.f} + Quaternion{1.f, 2.f, 3.f, 3.f} == Quaternion{});
    }

    SECTION("Subtraction") {
        REQUIRE(Quaternion{2.f, 4.f, 1.f, 3.f} - q1 == Quaternion{-1.f, -1.f, -1.f, -3.f});
        REQUIRE(Quaternion{} - q1 == Quaternion{-3.f, -5.f, -2.f, -6.f});
    }

    SECTION("Multiplication") {
        // a * b != b * a
        REQUIRE(Quaternion{1.f, 1.f, 1.f, 1.f} * q1 == Quaternion{6.f, 12.f, 10.f, -4.f});
        REQUIRE(q1 * Quaternion{1.f, 1.f, 1.f, 1.f} == Quaternion{12.f, 10.f, 6.f, -4.f});

        REQUIRE(Quaternion{} * q1 == Quaternion{}); // a * 0 = 0.
        REQUIRE(Quaternion::createUnit() * q1 == q1); // a * 1 = a.
        REQUIRE(q2 * q2Reversed == Quaternion{12.f, 24.f, 6.f, -12.f});
    }

    SECTION("Division") {
        REQUIRE(q2 / q2Reversed == Quaternion{-0.333333f, -0.666667f, 0.f, 0.666667f});
        REQUIRE(q2Reversed / q2 == Quaternion{0.333333f, 0.666667f, 0.f, 0.666667f});
        REQUIRE(q2 / Quaternion::createUnit() == q2);
        REQUIRE(q2 / Quaternion{} == Quaternion{});
    }
}

