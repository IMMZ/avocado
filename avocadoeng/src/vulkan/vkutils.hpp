#ifndef AVOCADO_VULKAN
#define AVOCADO_VULKAN

#include <vulkan/vulkan_core.h>

#include "internal/structuretypes.hpp" // todo rename this file.

#include <vector>

namespace avocado::vulkan {

class CommandBuffer;

template <typename T>
constexpr T createStruct() noexcept {
    static_assert(internal::StructureType<T> != VK_STRUCTURE_TYPE_MAX_ENUM, "This type is not supported");

    T t{};
    t.sType = internal::StructureType<T>;
    return t;
}

std::vector<VkCommandBuffer> getCommandBufferHandles(std::vector<CommandBuffer> &cmdBuffers);

template <typename T>
constexpr VkIndexType toIndexType() {
    static_assert(std::is_integral_v<T>
        && std::is_unsigned_v<T>
        && (sizeof(T) <= sizeof(uint32_t)),
        "Only unsigned type up to 32 bits is allowed.");

    if constexpr (sizeof(T) == sizeof(uint8_t))
        return VK_INDEX_TYPE_UINT8_EXT;
    if constexpr (sizeof(T) == sizeof(uint16_t))
        return VK_INDEX_TYPE_UINT16;
    if constexpr (sizeof(T) == sizeof(uint32_t))
        return VK_INDEX_TYPE_UINT32;
    return VK_INDEX_TYPE_MAX_ENUM;
}

}

#endif

