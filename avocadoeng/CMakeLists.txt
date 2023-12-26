project(avocado)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    "C:\\VulkanSDK\\1.3.211.0\\Include"
    "C:\\SDL2-2.0.22\\include"
    "C:\\SDL2_image-2.6.0\\include"
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

