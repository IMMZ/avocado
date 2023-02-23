#include "queue.hpp"

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

}

