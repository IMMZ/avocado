#include "../src/math/vecn.hpp"

#include <catch_amalgamated.hpp>

using namespace avocado::math;

TEST_CASE("Vector2f operations") {
    constexpr vec2f nullVec = vec2f::createNullVec();

    SECTION("Null vector") {
        constexpr vec2f notNullVec(2.f, 22.f);
        REQUIRE_FALSE(notNullVec.isNull());

        constexpr vec2f v1(12.f, 12.f);
        REQUIRE_FALSE(v1.isNull());

        REQUIRE(nullVec.isNull());
    }

    SECTION("Vector length") {
        constexpr vec2f v1(3.5f, 3.f);
        REQUIRE(avocado::core::areFloatsEq(v1.length(), 4.60977222f));
        REQUIRE_FALSE(v1.isUnit());

        constexpr auto v2 = vec2f::createNullVec();
        REQUIRE(avocado::core::areFloatsEq(v2.length(), 0.f));
        REQUIRE_FALSE(v2.isUnit());

        constexpr vec2f v3(0.f, 1.f);
        REQUIRE(avocado::core::areFloatsEq(v3.length(), 1.f));
        REQUIRE(v3.isUnit());
    }

    SECTION("Normalization") {
        avocado::math::vec2f v1(2.f, 22.f);
        REQUIRE_FALSE(v1.isUnit());
        v1.normalize();
        REQUIRE(v1.isUnit());

        avocado::math::vec2f v2(0.f, 1.f);
        REQUIRE(v2.isUnit());
        v2.normalize();
        REQUIRE(v2.isUnit());

        auto nullVec1 = vec2f::createNullVec();
        nullVec1.normalize();
        REQUIRE(nullVec1.isNull());
        REQUIRE(nullVec1 == vec2f::createNullVec());
        REQUIRE_FALSE(nullVec1.isUnit());
    }

    SECTION("Multiplication with scalar") {
        constexpr avocado::math::vec2f v1(2.f, 22.f);
        constexpr auto v2 = v1 * 3;
        constexpr auto v3 = -2.f * v1;
        REQUIRE(v2 == avocado::math::vec2f(6.f, 66.f));
        REQUIRE(v3 == avocado::math::vec2f(-4.f, -44.f));
        REQUIRE((v1 * 0).isNull());
        REQUIRE((v1 * 1) == v1);
    }

    SECTION("Sum") {
        constexpr avocado::math::vec2f v1(4.f, 2.f);
        constexpr avocado::math::vec2f v2(2.f, 3.f);

        constexpr auto v3 = v1 + v2;
        REQUIRE(v3 == avocado::math::vec2f(6.f, 5.f));

        constexpr auto v4 = v1 + nullVec;
        REQUIRE(v1 == v4);

        REQUIRE(v1 + v2 == v2 + v1);
    }

    SECTION("Subtraction") {
        constexpr avocado::math::vec2f v1{4.f, 2.f};
        constexpr avocado::math::vec2f v2{2.f, 3.f};

        constexpr auto v3 = v1 - v2;
        REQUIRE(v3 == avocado::math::vec2f{2.f, -1.f});

        constexpr auto v4 = v1 - vec2f::createNullVec();
        REQUIRE(v1 == v4);

        REQUIRE(v1 - v2 == -(v2 - v1));
    }

    SECTION("Scalar multiplication") {
        REQUIRE(vec2f{6.f, 5.f}.dotProduct(vec2f{6.f, 1.f}) > 0.f);
        REQUIRE(avocado::core::areFloatsEq(vec2f{0.f, 4.f}.dotProduct(vec2f{4.f, 0.f}), 0.f));
        REQUIRE(vec2f{-2.f, 5.f}.dotProduct(vec2f{6.f, 0.f}) < 0.f);
        vec2f v1{2.f, 3.f};
        REQUIRE(avocado::core::areFloatsEq(v1.dotProduct(v1), std::pow(v1.length(), 2.f)));

        vec2f v2 {5.f, 6.f};
        REQUIRE(avocado::core::areFloatsEq(v1.dotProduct(v2), v2.dotProduct(v1)));
    }

    SECTION("Scew product") {
        /*REQUIRE();
        REQUIRE();
        REQUIRE();*/
    }
}
