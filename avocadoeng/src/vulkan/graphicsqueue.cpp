#include "graphicsqueue.hpp"

#include "vkutils.hpp"

using namespace std::string_literals;

namespace avocado::vulkan {

GraphicsQueue::GraphicsQueue(VkQueue queue):
    Queue(queue) {
}

VkSubmitInfo GraphicsQueue::createSubmitInfo(const std::vector<VkSemaphore> &waitSemaphores, const std::vector<VkSemaphore> &signalSemaphores,
    const std::vector<VkCommandBuffer> &commandBuffers, const std::vector<VkPipelineStageFlags> &flags) {
    auto submitInfo = createStruct<VkSubmitInfo>();
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

void GraphicsQueue::submit(const VkSubmitInfo &submitInfo, VkFence fence) noexcept {
    assert(getHandle() != VK_NULL_HANDLE);

    const VkResult result = vkQueueSubmit(getHandle(), 1, &submitInfo, fence);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkQueueSubmit returned "s + getVkResultString(result));
    }
}

}
