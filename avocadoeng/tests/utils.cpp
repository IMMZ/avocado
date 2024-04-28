#include "../src/utils.hpp"
#include <catch_amalgamated.hpp>

TEST_CASE("String operations", "[utils]") {
    SECTION("Ends with") {
        REQUIRE_FALSE(avocado::utils::endsWith("", ""));
        REQUIRE_FALSE(avocado::utils::endsWith("hello", ""));
        REQUIRE_FALSE(avocado::utils::endsWith("", "hello"));
        REQUIRE(avocado::utils::endsWith("hello", "o"));
        REQUIRE(avocado::utils::endsWith("hello", "lo"));
        REQUIRE(avocado::utils::endsWith("hello", "llo"));
        REQUIRE(avocado::utils::endsWith("hello", "ello"));
        REQUIRE(avocado::utils::endsWith("hello", "hello"));
        REQUIRE_FALSE(avocado::utils::endsWith("hello", "a"));
        REQUIRE_FALSE(avocado::utils::endsWith("hello", "la"));
    }
}
