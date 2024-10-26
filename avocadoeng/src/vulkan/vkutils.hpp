#ifndef AVOCADO_VULKAN
#define AVOCADO_VULKAN

#include <vulkan/vulkan_core.h>

#include "structuretypes.hpp"

#include <vector>

namespace avocado::vulkan {

class Buffer;
class CommandBuffer;
class CommandPool;
class Image;
class Queue;

template <typename T>
constexpr VkIndexType toIndexType() {
    static_assert(std::is_integral_v<T>
        && std::is_unsigned_v<T>
        && (sizeof(T) <= sizeof(uint32_t)),
        "Only unsigned type up to 32 bits is allowed.");

    if constexpr (sizeof(T) == sizeof(uint8_t))
        return VK_INDEX_TYPE_UINT16;
    if constexpr (sizeof(T) == sizeof(uint16_t))
        return VK_INDEX_TYPE_UINT16;
    if constexpr (sizeof(T) == sizeof(uint32_t))
        return VK_INDEX_TYPE_UINT32;
    return VK_INDEX_TYPE_MAX_ENUM;
}

void copyBufferToImage(vulkan::CommandPool &commandPool, vulkan::Queue &queue, vulkan::Buffer &buffer,
    vulkan::Image &image, uint32_t width, uint32_t height);

void transitImageLayout(vulkan::CommandBuffer &cmdBuf, vulkan::Queue &queue, avocado::vulkan::Image &image, VkFormat format, VkImageLayout oldLayout,
    VkImageLayout newLayout, const VkImageAspectFlags aspectFlags);
}

#endif

