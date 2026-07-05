#include "buffer.hpp"

#include "image.hpp"
#include "logicaldevice.hpp"
#include "physicaldevice.hpp"

#include "vkutils.hpp"

#include <cstring>

using namespace std::string_literals;

namespace avocado::vulkan {

Buffer::Buffer(const VkDeviceSize size, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, LogicalDevice &logicalDevice,
    const std::vector<QueueFamily> &queueFamilies):
    _dev(logicalDevice.getHandle()),
    _bufSize(size) {
    DEFINE_VK_STRUCTURE(VkBufferCreateInfo, bufferCI);
    bufferCI.size = _bufSize;
    bufferCI.usage = usage;
    bufferCI.sharingMode = sharingMode;

    if (!queueFamilies.empty()) {
        bufferCI.queueFamilyIndexCount = static_cast<decltype(bufferCI.queueFamilyIndexCount)>(queueFamilies.size());
        bufferCI.pQueueFamilyIndices = queueFamilies.data();
    }

    CALL_VULKAN(vkCreateBuffer, _dev, &bufferCI, nullptr, &_buf);
}

Buffer::Buffer(Buffer &&other):
    _dev(std::move(other._dev))
    , _buf(std::move(other._buf))
    , _devMem(std::move(other._devMem))
    , _bufSize(std::move(other._bufSize)) {
    other._dev = VK_NULL_HANDLE;
    other._buf = VK_NULL_HANDLE;
    other._devMem = VK_NULL_HANDLE;
    other._bufSize = 0;
}

Buffer& Buffer::operator=(Buffer &&other) {
    _dev = std::move(other._dev);
    _buf = std::move(other._buf);
    _devMem = std::move(other._devMem);
    _bufSize = std::move(other._bufSize);

    other._dev = VK_NULL_HANDLE;
    other._buf = VK_NULL_HANDLE;
    other._devMem = VK_NULL_HANDLE;
    other._bufSize = 0;

    return *this;
}

Buffer::~Buffer() {
    if (_buf != VK_NULL_HANDLE)
        vkDestroyBuffer(_dev, _buf, nullptr);
    if (_devMem != VK_NULL_HANDLE)
        vkFreeMemory(_dev, _devMem, nullptr);
}

VkBuffer Buffer::getHandle() noexcept {
    return _buf;
}

void Buffer::allocateMemory(PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags) {
    assert(_buf != VK_NULL_HANDLE && _dev != VK_NULL_HANDLE && "Buffer and device mustn't be null.");
    assert(_devMem == VK_NULL_HANDLE && "Device memory mustn't be null.");

    // Memory requirements.
    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(_dev, _buf, &memReq);

    const uint32_t foundType = physDevice.findMemoryTypeIndex(memoryFlags, memReq.memoryTypeBits);

    // Allocate memory.
    DEFINE_VK_STRUCTURE(VkMemoryAllocateInfo, memAllocInfo);
    memAllocInfo.allocationSize = memReq.size;
    memAllocInfo.memoryTypeIndex = foundType;

    CALL_VULKAN(vkAllocateMemory, _dev, &memAllocInfo, nullptr, &_devMem);
}

void Buffer::bindMemory(const VkDeviceSize offset) noexcept {
    assert(_dev != VK_NULL_HANDLE && _buf != VK_NULL_HANDLE && _devMem != VK_NULL_HANDLE && "Handles mustn't be null.");

    CALL_VULKAN(vkBindBufferMemory, _dev, _buf, _devMem, offset);
}

void Buffer::fill(const void * const dataToCopy, const VkDeviceSize dataSize, const size_t offset) {
    assert(_dev != VK_NULL_HANDLE && _devMem != VK_NULL_HANDLE && "Handles mustn't be null.");

    void *data = nullptr;
    CALL_VULKAN_AND_RETURN(vkMapMemory, _dev, _devMem, offset, dataSize, 0, &data);
    memcpy(data, dataToCopy, dataSize);
    vkUnmapMemory(_dev, _devMem);
}

VkDeviceSize Buffer::getSizeBytes() const noexcept {
    return _bufSize;
}

}

