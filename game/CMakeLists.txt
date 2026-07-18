project(game)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
set(AVOCADO_SRC_DIR ${PROJECT_SOURCE_DIR}/../avocadoeng/src)

include_directories(
    "../avocadoeng/src"
    ${VULKAN_SDK_INCLUDE_DIRECTORY}
    "/usr/include/SDL2/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2"
)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}_tests
    src/gameconfig.cpp
    ${TEST_SOURCES}
    ${AVOCADO_SRC_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2/catch_amalgamated.cpp)

if (ASAN_ENABLED)
	target_compile_options(${PROJECT_NAME} PRIVATE
	    -fsanitize=address
	    -fno-omit-frame-pointer
	    -g
	    -O0)

	target_link_options(${PROJECT_NAME}
	PRIVATE -fsanitize=address)
endif()


target_link_libraries(${PROJECT_NAME}
    avocadoengine
    SDL2
    SDL2main
    SDL2_image
    ${VULKAN_SDK_LIBRARY}
)

