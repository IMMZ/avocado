#include <cstdint>
#include <string>

struct Config {
    static constexpr const char * GAME_NAME = "Some game";
    static constexpr uint32_t GAME_MAJOR_VERSION = 0;
    static constexpr uint32_t GAME_MINOR_VERSION = 1;
    static constexpr uint32_t GAME_PATCH_VERSION = 0;
    static inline const std::string SHADERS_PATH = "assets/shaders";
    static constexpr int RESOLUTION_WIDTH = 800;
    static constexpr int RESOLUTION_HEIGHT = 600;
};
