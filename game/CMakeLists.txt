project(game)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    "C:\\thegame\\avocadoeng\\src"
    "C:\\VulkanSDK\\1.3.211.0\\Include"
    "C:\\SDL2-2.0.22\\include"
    "C:\\SDL2_image-2.6.0\\include"
)

link_directories(
    "C:\\thegame\\avocadoeng\\Debug"
    "C:\\VulkanSDK\\1.3.211.0\\Lib"
    "C:\\SDL2-2.0.22\\lib\\x64"
    "C:\\SDL2_image-2.6.0\\lib\\x64"
)

add_executable(game src/game.cpp)

target_link_libraries(game
    avocado
    SDL2
    SDL2main
    SDL2_image
    vulkan-1
)
