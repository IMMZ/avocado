#ifndef AVOCADO_VULKAN_BUFFER
#define AVOCADO_VULKAN_BUFFER

#include "types.hpp"

#include "../errorstorage.hpp"
#include "../utils.hpp"

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

class CommandBuffer;
class Image;
class LogicalDevice;
class PhysicalDevice;

class Buffer: public core::ErrorStorage {
public:
    NON_COPYABLE(Buffer);

    explicit Buffer(const VkDeviceSize size, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, LogicalDevice &device, const std::vector<QueueFamily> &queueFamilies = {});
    Buffer(Buffer &&other);
    Buffer& operator=(Buffer &&other);
    ~Buffer();

    VkBuffer getHandle() noexcept;
    void allocateMemory(PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags);
    void bindMemory(const VkDeviceSize offset = 0) noexcept;
    void copyToImage(Image &image, const uint32_t width, const uint32_t height, CommandBuffer &commandBuffer);
    void fill(const void * const dataToCopy, const VkDeviceSize dataSize, const size_t offset = 0);
    inline void fill(const void * const dataToCopy) {
        fill(dataToCopy, _bufSize, 0);
    }
    VkDeviceSize getSize() const noexcept;

private:
    VkDevice _dev = VK_NULL_HANDLE;
    VkBuffer _buf = VK_NULL_HANDLE;
    VkDeviceMemory _devMem = VK_NULL_HANDLE;
    VkDeviceSize _bufSize = 0;
};

} // namespace avocado::vulkan.

#endif

