#ifndef GAMECONFIG_HPP
#define GAMECONFIG_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

class GameConfig {
public:
    static constexpr const char * GAME_NAME = "Some engine";
    static constexpr uint32_t GAME_MAJOR_VERSION = 0;
    static constexpr uint32_t GAME_MINOR_VERSION = 1;
    static constexpr uint32_t GAME_PATCH_VERSION = 0;
    static constexpr int RESOLUTION_WIDTH = 800;
    static constexpr int RESOLUTION_HEIGHT = 600;

    [[nodiscard]] bool load(const std::string_view &path);
    [[nodiscard]] bool save();
    [[nodiscard]] const std::string& getValue(const std::string &key) const;
    [[nodiscard]] bool hasValue(const std::string &key) const;
    void setValue(const std::string &key, const std::string &value);
    void setValue(const std::string &key, std::string &&value);
    [[nodiscard]] static std::string getShadersPath();

private:
    std::unordered_map<std::string /* key */, std::string /* value */> _map;
    std::string _fileName;
};

#endif // GAMECONFIG_HPP

