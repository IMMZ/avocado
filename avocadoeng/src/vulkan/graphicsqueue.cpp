#include "graphicsqueue.hpp"

#include "vkutils.hpp"

using namespace std::string_literals;

namespace avocado::vulkan {

GraphicsQueue::GraphicsQueue(VkQueue queue):
    Queue(queue),
    _submitInfo(createStruct<VkSubmitInfo>()) {
}

void GraphicsQueue::setSemaphores(const std::vector<VkSemaphore> &waitSemaphores, const std::vector<VkSemaphore> &signalSemaphores) {
    _submitInfo.waitSemaphoreCount = static_cast<decltype(_submitInfo.waitSemaphoreCount)>(waitSemaphores.size());
    _submitInfo.pWaitSemaphores = waitSemaphores.data();
    _submitInfo.signalSemaphoreCount = static_cast<decltype(_submitInfo.signalSemaphoreCount)>(signalSemaphores.size());
    _submitInfo.pSignalSemaphores = signalSemaphores.data();

}

void GraphicsQueue::setCommandBuffers(std::vector<VkCommandBuffer> &commandBuffers) {
    _submitInfo.commandBufferCount = static_cast<decltype(_submitInfo.commandBufferCount)>(commandBuffers.size());
    _submitInfo.pCommandBuffers = commandBuffers.data();
}

void GraphicsQueue::setPipelineStageFlags(const std::vector<PipelineStageFlag> &flags) {
    _pipelineStageFlags.resize(flags.size());
    for (size_t i = 0; i < flags.size(); ++i) {
        _pipelineStageFlags[i] = static_cast<decltype(_pipelineStageFlags)::value_type>(flags[i]);
    }
    _submitInfo.pWaitDstStageMask = _pipelineStageFlags.data();
}

void GraphicsQueue::submit(VkFence fence) {
    assert(getHandle() != VK_NULL_HANDLE);

    const VkResult result = vkQueueSubmit(getHandle(), 1, &_submitInfo, fence);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkQueueSubmit returned "s + getVkResultString(result));
    }
}

}
