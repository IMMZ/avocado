#include "../src/utils.hpp"

#include <catch_amalgamated.hpp>

#include <vector>

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

    SECTION("Contains") {
        std::string emptyString;
        std::string testString = "My test string?";
        REQUIRE_FALSE(avocado::utils::stringContains(emptyString, "A"));
        REQUIRE_FALSE(avocado::utils::stringContains(emptyString, "a"));
        REQUIRE(avocado::utils::stringContains(testString, "test"));
        REQUIRE(avocado::utils::stringContains(testString, " "));
        REQUIRE(avocado::utils::stringContains(testString, "?"));
        REQUIRE(avocado::utils::stringContains(testString, "My test string?"));
        REQUIRE(avocado::utils::stringContains(testString, emptyString));
        REQUIRE_FALSE(avocado::utils::stringContains(testString, "my"));
        REQUIRE_FALSE(avocado::utils::stringContains(testString, "ttest"));
        REQUIRE_FALSE(avocado::utils::stringContains(testString, "!"));
        REQUIRE_FALSE(avocado::utils::stringContains(testString, "My_test string?"));
    }

    SECTION("Splitting") {
        std::string emptyString;
        std::string testString = "Try:to:split:this::string";
        const std::vector okVector = avocado::utils::splitString(testString, ":", false);
        REQUIRE(okVector[0] == "Try");
        REQUIRE(okVector[1] == "to");
        REQUIRE(okVector[2] == "split");
        REQUIRE(okVector[3] == "this");
        REQUIRE(okVector[4] == "string");

        const std::vector splitByInvalidSeparatorResult = avocado::utils::splitString(testString, "_", false);
        REQUIRE(splitByInvalidSeparatorResult.front() == testString);

        const std::vector okVectorWithEmpty = avocado::utils::splitString(testString, ":", true);
        REQUIRE(okVectorWithEmpty[0] == "Try");
        REQUIRE(okVectorWithEmpty[1] == "to");
        REQUIRE(okVectorWithEmpty[2] == "split");
        REQUIRE(okVectorWithEmpty[3] == "this");
        REQUIRE(okVectorWithEmpty[4] == std::string());
        REQUIRE(okVectorWithEmpty[5] == "string");

        const std::vector splitEmptyResult = avocado::utils::splitString(emptyString, "_", false);
        REQUIRE(splitEmptyResult.empty());
        const std::vector splitEmptyWithEmptyResult = avocado::utils::splitString(emptyString, "_", true);
        REQUIRE(splitEmptyWithEmptyResult.size() == 1);
        REQUIRE(splitEmptyWithEmptyResult.front() == emptyString);
    }
}

