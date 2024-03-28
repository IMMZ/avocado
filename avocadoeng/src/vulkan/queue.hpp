#ifndef AVOCADO_VULKAN_QUEUE
#define AVOCADO_VULKAN_QUEUE

#include "../errorstorage.hpp"

#include <vector>

namespace avocado::vulkan {

class CommandBuffer;

struct VkQueue_T;

class Queue: public core::ErrorStorage {
public:
    explicit Queue(VkQueue q);
    virtual ~Queue() = default;

    VkQueue getHandle() noexcept;
    void waitIdle() noexcept;

    VkSubmitInfo createSubmitInfo(VkSemaphore &waitSemaphore, VkSemaphore &signalSemaphore, VkCommandBuffer &commandBuffer,
        const std::vector<VkPipelineStageFlags> &flags);
    void submit(const VkSubmitInfo &submitInfo, VkFence fence = VK_NULL_HANDLE) noexcept;
    void present(VkSemaphore &waitSemaphore, uint32_t &imageIndex, VkSwapchainKHR &swapchain);

private:
    VkQueue _queue = VK_NULL_HANDLE;
};

}

#endif

