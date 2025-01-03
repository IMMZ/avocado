# Try to find VULKAN_SDK environment variable at first.
# This variable is set by script from Vulkan SDK.
set(VULKAN_SDK $ENV{VULKAN_SDK})
if("${VULKAN_SDK}" STREQUAL "")
    message(FATAL_ERROR "VULKAN_SDK environment is not set")
endif()

if (NOT EXISTS ${VULKAN_SDK}/include)
    message(FATAL_ERROR "Can't find SDK include directory")
endif()

if (NOT EXISTS ${VULKAN_SDK}/lib)
    message(FATAL_ERROR "Can't find SDK library directory")
endif()

set(VULKAN_SDK_INCLUDE_DIRECTORY ${VULKAN_SDK}/include)

find_path(VULKAN_SDK_LIBRARY_PATH NAMES libvulkan.so vulkan-1.lib PATHS ${VULKAN_SDK}/lib)
link_directories(${VULKAN_SDK_LIBRARY_PATH})
set(VULKAN_SDK_LIBRARY vulkan)

