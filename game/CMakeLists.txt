project(game)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(
    "../avocadoeng/src"
    "/usr/include/vulkan"
    "/usr/include/SDL2/"
)


file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

target_link_libraries(${CMAKE_PROJECT_NAME}
    avocado
    SDL2
    SDL2main
    SDL2_image
    vulkan
)
