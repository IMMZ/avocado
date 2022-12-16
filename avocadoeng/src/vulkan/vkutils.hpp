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

}

#endif

