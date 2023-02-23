#ifndef AVOCADO_VULKAN_GRAPHICS_QUEUE
#define AVOCADO_VULKAN_GRAPHICS_QUEUE

#include "queue.hpp"

namespace avocado::vulkan {

class GraphicsQueue final: public Queue {
public:
    explicit GraphicsQueue(VkQueue queue);

    VkSubmitInfo createSubmitInfo(const std::vector<VkSemaphore> &waitSemaphores, const std::vector<VkSemaphore> &signalSemaphores,
        const std::vector<VkCommandBuffer> &commandBuffers, const std::vector<VkPipelineStageFlags> &flags);
    void submit(const VkSubmitInfo &submitInfo, VkFence fence = VK_NULL_HANDLE) noexcept;
};

}

#endif

