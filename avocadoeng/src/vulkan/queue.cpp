#include "queue.hpp"

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

    const VkResult res = vkQueueWaitIdle(_queue);
    setHasError(res != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkQueueWaitIdle returned "s + getVkResultString(res));
}

VkSubmitInfo Queue::createSubmitInfo(VkSemaphore &waitSemaphore, VkSemaphore &signalSemaphore,
    VkCommandBuffer &commandBuffer, const std::vector<VkPipelineStageFlags> &flags) {
    VkSubmitInfo submitInfo{};
    FILL_S_TYPE(submitInfo);

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

    const VkResult result = vkQueueSubmit(getHandle(), 1, &submitInfo, fence);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkQueueSubmit returned "s + getVkResultString(result));
    }
}

void Queue::present(VkSemaphore &waitSemaphore, uint32_t &imageIndex, VkSwapchainKHR &swapchain) {
    VkPresentInfoKHR presentInfo{}; FILL_S_TYPE(presentInfo);

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;

    presentInfo.pImageIndices = &imageIndex;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;

    const VkResult result = vkQueuePresentKHR(getHandle(), &presentInfo);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkQueuePresentKHR returned "s + getVkResultString(result));
    }
}

}

