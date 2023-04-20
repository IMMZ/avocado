#include "../src/core.hpp"
#include <catch_amalgamated.hpp>

TEST_CASE("Core operations", "[core]") {
    SECTION("Comparing floats") {
        REQUIRE(avocado::core::areFloatsEq(1.0, 1.0));
        REQUIRE_FALSE(avocado::core::areFloatsEq(1.0, 1.0001));
    }
}
