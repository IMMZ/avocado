project(avocadoengine)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

include_directories(
    "./src"
    ${VULKAN_SDK}/include
    "/usr/include/SDL2/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2"
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
file(GLOB_RECURSE SHADERS ../assets/shaders/*.*)

# Shader compilation stuff.
foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/shaders/${SHADER_NAME}.spv)

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${VULKAN_SDK}/bin/glslc ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME}..."
    )

    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${SHADER_OUTPUTS})

add_library(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}_tests
    src/math/functions.cpp
    src/math/quaternion.cpp
    src/utils.cpp
    ${TEST_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2/catch_amalgamated.cpp)

