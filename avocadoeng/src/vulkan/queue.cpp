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
    assert(_queue != VK_NULL_HANDLE);

    const VkResult res = vkQueueWaitIdle(_queue);
    setHasError(res != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkQueueWaitIdle returned "s + getVkResultString(res));
}

VkSubmitInfo Queue::createSubmitInfo(const std::vector<VkSemaphore> &waitSemaphores, const std::vector<VkSemaphore> &signalSemaphores,
    const std::vector<VkCommandBuffer> &commandBuffers, const std::vector<VkPipelineStageFlags> &flags) {
    VkSubmitInfo submitInfo{};
    FILL_S_TYPE(submitInfo);

    if (!waitSemaphores.empty()) {
        submitInfo.waitSemaphoreCount = static_cast<decltype(submitInfo.waitSemaphoreCount)>(waitSemaphores.size());
        submitInfo.pWaitSemaphores = waitSemaphores.data();
    }

    if (!signalSemaphores.empty()) {
        submitInfo.signalSemaphoreCount = static_cast<decltype(submitInfo.signalSemaphoreCount)>(signalSemaphores.size());
        submitInfo.pSignalSemaphores = signalSemaphores.data();
    }

    if (!commandBuffers.empty()) {
        submitInfo.commandBufferCount = static_cast<decltype(submitInfo.commandBufferCount)>(commandBuffers.size());
        submitInfo.pCommandBuffers = commandBuffers.data();
    }

    if (!flags.empty())
        submitInfo.pWaitDstStageMask = flags.data();

    return submitInfo;
}

void Queue::submit(const VkSubmitInfo &submitInfo, VkFence fence) noexcept {
    assert(getHandle() != VK_NULL_HANDLE);

    const VkResult result = vkQueueSubmit(getHandle(), 1, &submitInfo, fence);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkQueueSubmit returned "s + getVkResultString(result));
    }
}

void Queue::present(const std::vector<VkSemaphore> &waitSemaphores,
    const std::vector<uint32_t> &imageIndices,
    std::vector<VkSwapchainKHR> &swapchains) {
    VkPresentInfoKHR presentInfo{}; FILL_S_TYPE(presentInfo);

    presentInfo.waitSemaphoreCount = static_cast<decltype(presentInfo.waitSemaphoreCount)>(waitSemaphores.size());
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    presentInfo.pImageIndices = imageIndices.data();

    presentInfo.swapchainCount = static_cast<decltype(presentInfo.swapchainCount)>(swapchains.size());
    presentInfo.pSwapchains = swapchains.data();

    const VkResult result = vkQueuePresentKHR(getHandle(), &presentInfo);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkQueuePresentKHR returned "s + getVkResultString(result));
    }
}

}

