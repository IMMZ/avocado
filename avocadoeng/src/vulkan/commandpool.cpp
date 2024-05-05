#include "commandpool.hpp"

#include "buffer.hpp"
#include "image.hpp"
#include "logicaldevice.hpp"
#include "queue.hpp"
#include "structuretypes.hpp"
#include "swapchain.hpp"

namespace avocado::vulkan {

CommandPool::CommandPool(LogicalDevice &device, const VkCommandPoolCreateFlags flags, const QueueFamily queueFamilyIndex):
    _device(device) {
    VkCommandPoolCreateInfo poolCreateInfo{}; FILL_S_TYPE(poolCreateInfo);
    poolCreateInfo.flags = flags;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    const VkResult result = vkCreateCommandPool(_device.getHandle(), &poolCreateInfo, nullptr, &_pool);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkCreateCommandPool returned "s + getVkResultString(result));
}

CommandPool::~CommandPool() {
    if (_pool != VK_NULL_HANDLE)
        vkDestroyCommandPool(_device.getHandle(), _pool, nullptr); // All allocated buffers are freed automatically.
}

size_t CommandPool::allocateBuffers(const uint32_t count, const VkCommandBufferLevel bufferLevel) {
    assert(_pool != VK_NULL_HANDLE && "Null command pool.");
    assert(count > 0 && "Command buffer count must be > 0.");

    VkCommandBufferAllocateInfo allocInfo{}; FILL_S_TYPE(allocInfo);
    allocInfo.commandPool = _pool;
    allocInfo.level = static_cast<VkCommandBufferLevel>(bufferLevel);
    allocInfo.commandBufferCount = count;

    const size_t oldSize = _buffers.size();
    _buffers.resize(_buffers.size() + count);
    const VkResult callResult = vkAllocateCommandBuffers(_device.getHandle(), &allocInfo, _buffers.data() + oldSize);
    setHasError(callResult != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkAllocateCommandBuffers returned "s + getVkResultString(callResult));

    return oldSize; // old size = index of 1st allocated buffer.
}

void CommandPool::freeBuffers(const size_t indexFrom, const size_t indexTo) {
    assert(indexTo >= indexFrom && "indexFrom must be <= indexTo");
    vkFreeCommandBuffers(_device.getHandle(), _pool, indexTo - indexFrom + 1, _buffers.data() + indexFrom);
    _buffers.erase(_buffers.cbegin() + indexFrom, _buffers.cbegin() + indexTo);
}

void CommandPool::freeBuffers() {
    vkFreeCommandBuffers(_device.getHandle(), _pool, _buffers.size(), _buffers.data());
    _buffers.clear();
}

size_t CommandPool::getBuffersCount() const noexcept {
    return _buffers.size();
}

VkCommandBuffer* CommandPool::getBuffers() noexcept {
    return _buffers.data();
}

CommandBuffer CommandPool::getBuffer(const size_t index) noexcept {
    return CommandBuffer(_buffers[index]);
}

VkCommandBuffer& CommandPool::getFirstBuffer() noexcept {
    return _buffers.front();
}

VkCommandBuffer& CommandPool::getLastBuffer() noexcept {
    return _buffers.back();
}

VkCommandPool& CommandPool::getPool() noexcept {
    return _pool;
}

} // namespace avocado::vulkan.

