#include "presentqueue.hpp"

#include "vkutils.hpp"

using namespace std::string_literals;

namespace avocado::vulkan {

PresentQueue::PresentQueue(VkQueue queue):
    Queue(queue),
    _presentInfo(createStruct<VkPresentInfoKHR>()){
}

void PresentQueue::setWaitSemaphores(const std::vector<VkSemaphore> &waitSemaphores) noexcept {
    _presentInfo.waitSemaphoreCount = static_cast<decltype(_presentInfo.waitSemaphoreCount)>(waitSemaphores.size());
    _presentInfo.pWaitSemaphores = waitSemaphores.data();
}

void PresentQueue::setSwapchains(std::vector<Swapchain> &swapchains) {
    _swapchainHandles.resize(swapchains.size(), VK_NULL_HANDLE);
    for (size_t i = 0; i < swapchains.size(); ++i) {
        _swapchainHandles[i] = swapchains[i].getHandle();
    }
    _swapchainHandles.shrink_to_fit();

    _presentInfo.swapchainCount = static_cast<decltype(_presentInfo.swapchainCount)>(_swapchainHandles.size());
    _presentInfo.pSwapchains = _swapchainHandles.data();
}

void PresentQueue::setImageIndices(const std::vector<uint32_t> &imageIndices) noexcept {
    _presentInfo.pImageIndices = imageIndices.data();
}

void PresentQueue::present() noexcept {
    const VkResult result = vkQueuePresentKHR(getHandle(), &_presentInfo);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkQueuePresentKHR returned "s + getVkResultString(result));
    }
}

}

