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
    VkSubmitInfo createSubmitInfo(const std::vector<VkSemaphore> &waitSemaphores,
        const std::vector<VkSemaphore> &signalSemaphores,
        const std::vector<VkCommandBuffer> &commandBuffers,
        const std::vector<VkPipelineStageFlags> &flags);
    void submit(const VkSubmitInfo &submitInfo, VkFence fence = VK_NULL_HANDLE) noexcept;
    void present(const std::vector<VkSemaphore> &waitSemaphores,
        const std::vector<uint32_t> &imageIndices,
        std::vector<VkSwapchainKHR> &swapchains);

private:
    VkQueue _queue = VK_NULL_HANDLE;
};

}

#endif

