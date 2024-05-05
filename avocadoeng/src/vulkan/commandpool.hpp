#ifndef AVOCADO_VULKAN_COMMAND_POOL
#define AVOCADO_VULKAN_COMMAND_POOL

#include "../errorstorage.hpp"

#include "commandbuffer.hpp"
#include "types.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace avocado::vulkan {

class Buffer;
class Image;
class LogicalDevice;
class Queue;
class Swapchain;

class CommandPool final: public core::ErrorStorage {
public:
    explicit CommandPool(LogicalDevice &device, const VkCommandPoolCreateFlags flags, const QueueFamily queueFamilyIndex);
    ~CommandPool();

    // Disable to copy.
    // todo make a macro
    CommandPool(const CommandPool &) = delete;
    CommandPool& operator=(const CommandPool&) = delete;

    size_t allocateBuffers(const uint32_t count, const VkCommandBufferLevel bufferLevel);
    void freeBuffers(const size_t indexFrom, const size_t indexTo);
    inline void freeBuffer(const size_t index) { freeBuffers(index, index); }
    void freeBuffers();
    size_t getBuffersCount() const noexcept;
    VkCommandBuffer* getBuffers() noexcept;
    CommandBuffer getBuffer(const size_t index) noexcept;
    VkCommandBuffer& getFirstBuffer() noexcept;
    VkCommandBuffer& getLastBuffer() noexcept;
    VkCommandPool& getPool() noexcept;

private:
    std::vector<VkCommandBuffer> _buffers;
    VkCommandPool _pool = VK_NULL_HANDLE;
    LogicalDevice &_device;
};

}
#endif /* ifndef AVOCADO_VULKAN_COMMAND_POOL */
