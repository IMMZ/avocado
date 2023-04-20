project(avocado)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    "C:\\VulkanSDK\\1.3.211.0\\Include"
    "C:\\SDL2-2.0.22\\include"
    "C:\\SDL2_image-2.6.0\\include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2"
)

add_library(avocado
    src/errorstorage.cpp
    src/vulkantools.cpp
    src/utils.cpp

    src/math/functions.cpp
    src/math/quaternion.cpp

    src/vulkan/buffer.cpp
    src/vulkan/commandbuffer.cpp
    src/vulkan/debugutils.cpp
    src/vulkan/graphicspipeline.cpp
    src/vulkan/logicaldevice.cpp
    src/vulkan/physicaldevice.cpp
    src/vulkan/queue.cpp
    src/vulkan/surface.cpp
    src/vulkan/swapchain.cpp
    src/vulkan/vkutils.cpp

    src/vulkan/states/colorblendstate.cpp
    src/vulkan/states/dynamicstate.cpp
    src/vulkan/states/inputasmstate.cpp
    src/vulkan/states/multisamplestate.cpp
    src/vulkan/states/rasterizationstate.cpp
    src/vulkan/states/vertexinputstate.cpp
    src/vulkan/states/viewportstate.cpp)

add_executable(avocado_tests
    src/math/functions.cpp
    src/math/quaternion.cpp

    tests/core.cpp
    tests/mathfunctions.cpp
    tests/matrix.cpp
    tests/quaternion.cpp
    tests/vecn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Catch2-3.3.2/catch_amalgamated.cpp)

