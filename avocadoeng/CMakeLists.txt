project(avocadoengine)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    "/usr/include/vulkan"
    "/usr/include/SDL2/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2"
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}_tests
    src/math/functions.cpp
    src/math/quaternion.cpp
    src/utils.cpp
    ${TEST_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2/catch_amalgamated.cpp)

