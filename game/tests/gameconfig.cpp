#include "../src/gameconfig.hpp"

#include <catch_amalgamated.hpp>

#include <fstream>

namespace {
    [[nodiscard]] bool writeFile(const std::string &filename, const std::string &content) {
        std::ofstream outFile(filename, std::ios_base::out | std::ios_base::trunc);
        if (outFile.is_open()) {
            outFile << content;
            outFile.close();
            return true;
        }

        return false;
    }
}

struct GameConfigFixture {
    const std::string fileName = "/tmp/testConfig"; // todo make crossplatform path
    const std::string testConfig = "=valueWithEmptyKey\n"
            "keyWithEmptyValue=\n"
            "someKey=someValue\n"
            "someKeySomeValue\n"
            "someKey1=some=Value1\n"
            "someKey2==someValue2\n"
            "=someKey3=someValue3=\n"
            "#someKey4=someValue4\n";
};

TEST_CASE_METHOD(GameConfigFixture, "Work with config", "[general]") {
    SECTION("Load and read") {
        const bool writeResult = writeFile(fileName, testConfig);
        REQUIRE(writeResult == true);

        GameConfig gameConfig;
        const bool loadResult = gameConfig.load(fileName);
        REQUIRE(loadResult == true);
        REQUIRE(gameConfig.getValue("") == "someKey3=someValue3=");
        REQUIRE(gameConfig.getValue("keyWithEmptyValue").empty());
        REQUIRE(gameConfig.getValue("someKey") == "someValue");
        REQUIRE_FALSE(gameConfig.hasValue("someKeySomeValue"));
        REQUIRE(gameConfig.getValue("someKey1") == "some=Value1");
        REQUIRE(gameConfig.getValue("someKey2") == "=someValue2");
        REQUIRE_FALSE(gameConfig.hasValue("someKey4"));
    }

    SECTION("Modify and save") {
        const bool writeResult = writeFile(fileName, testConfig);
        REQUIRE(writeResult == true);

        GameConfig gameConfig;
        const bool loadResult = gameConfig.load(fileName);
        REQUIRE(loadResult == true);

        gameConfig.setValue("", "");
        REQUIRE(gameConfig.getValue("") == "");
        gameConfig.setValue("keyWithEmptyValue", "nonEmptyValue");
        REQUIRE(gameConfig.getValue("keyWithEmptyValue") == "nonEmptyValue");
        gameConfig.setValue("someKey1", "someValue1");
        REQUIRE(gameConfig.getValue("someKey1") == "someValue1");
        gameConfig.setValue("someKey2", "someValue2");
        REQUIRE(gameConfig.getValue("someKey2") == "someValue2");
        gameConfig.setValue("someKey4", "someValue4");
        REQUIRE(gameConfig.getValue("someKey4") == "someValue4");

        const bool saveResult = gameConfig.save();
        REQUIRE(saveResult == true);
        const bool load1Result = gameConfig.load(fileName);
        REQUIRE(load1Result == true);
        REQUIRE(gameConfig.getValue("") == "");
        REQUIRE(gameConfig.getValue("keyWithEmptyValue") == "nonEmptyValue");
        REQUIRE(gameConfig.getValue("someKey1") == "someValue1");
        REQUIRE(gameConfig.getValue("someKey2") == "someValue2");
        REQUIRE(gameConfig.getValue("someKey4") == "someValue4");
    }
}
