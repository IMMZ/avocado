#include "vkutils.hpp"

#include "commandbuffer.hpp"

#include <algorithm>

namespace avocado::vulkan {

std::vector<VkCommandBuffer> getCommandBufferHandles(std::vector<CommandBuffer> &cmdBuffers) {
    std::vector<VkCommandBuffer> result(cmdBuffers.size(), VK_NULL_HANDLE);
    std::transform(cmdBuffers.begin(), cmdBuffers.end(), result.begin(), [](CommandBuffer &cmdBuf) {
        return cmdBuf.getHandle();
    });

    return result;
}

}

