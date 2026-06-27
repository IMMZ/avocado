#include "queue.hpp"

#include "../utils.hpp"
#include "vkutils.hpp"

#include <vulkan/vulkan_core.h>

using namespace std::string_literals;

namespace avocado::vulkan {

Queue::Queue(VkQueue vq):
    _queue(vq) {
}

VkQueue Queue::getHandle() noexcept {
    return _queue;
}

void Queue::waitIdle() noexcept {
    assert(_queue != VK_NULL_HANDLE && "Queue handle mustn't be null.");

    CALL_VULKAN(vkQueueWaitIdle, _queue);
}

VkSubmitInfo Queue::createSubmitInfo(VkSemaphore &waitSemaphore, VkSemaphore &signalSemaphore,
    VkCommandBuffer &commandBuffer, const std::vector<VkPipelineStageFlags> &flags) {
    DEFINE_VK_STRUCTURE(VkSubmitInfo, submitInfo);
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (!flags.empty())
        submitInfo.pWaitDstStageMask = flags.data();

    return submitInfo;
}

void Queue::submit(const VkSubmitInfo &submitInfo, VkFence fence) noexcept {
    assert((getHandle() != VK_NULL_HANDLE) && "Queue handle mustn't be null.");

    CALL_VULKAN(vkQueueSubmit, getHandle(), 1 /* submitCount */, &submitInfo, fence);
}

void Queue::present(VkSemaphore &waitSemaphore, uint32_t &imageIndex, VkSwapchainKHR &swapchain) {
    DEFINE_VK_STRUCTURE(VkPresentInfoKHR, presentInfo);

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;

    presentInfo.pImageIndices = &imageIndex;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;

    CALL_VULKAN(vkQueuePresentKHR, getHandle(), &presentInfo);
}

}

