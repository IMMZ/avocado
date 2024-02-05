project(avocado)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    "/usr/include/vulkan"
    "/usr/include/SDL2/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2"
)

file(GLOB_RECURSE SOURCES *.cpp)

add_library(avocado ${SOURCES})
add_executable(avocado_tests
    src/math/functions.cpp
    src/math/quaternion.cpp

    tests/core.cpp
    tests/mathfunctions.cpp
    tests/matrix.cpp
    tests/quaternion.cpp
    tests/vecn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2/catch_amalgamated.cpp)

