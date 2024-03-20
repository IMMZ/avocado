#include "buffer.hpp"

#include "commandbuffer.hpp"
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
    VkBufferCreateInfo bufferCI{}; FILL_S_TYPE(bufferCI);
    bufferCI.size = _bufSize;
    bufferCI.usage = usage;
    bufferCI.sharingMode = sharingMode;

    if (!queueFamilies.empty()) {
        bufferCI.queueFamilyIndexCount = static_cast<decltype(bufferCI.queueFamilyIndexCount)>(queueFamilies.size());
        bufferCI.pQueueFamilyIndices = queueFamilies.data();
    }

    const VkResult result = vkCreateBuffer(_dev, &bufferCI, nullptr, &_buf);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkCreateBuffer returned "s + getVkResultString(result));
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
    core::ErrorStorage::operator=(other);

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
    if (_devMem != VK_NULL_HANDLE)
        vkFreeMemory(_dev, _devMem, nullptr);
    if (_buf != VK_NULL_HANDLE)
        vkDestroyBuffer(_dev, _buf, nullptr);
}

VkBuffer Buffer::getHandle() noexcept {
    return _buf;
}

void Buffer::allocateMemory(PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags) {
    assert(_buf != VK_NULL_HANDLE && _dev != VK_NULL_HANDLE);
    assert(_devMem == VK_NULL_HANDLE);

    // Memory requirements.
    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(_dev, _buf, &memReq);

    const uint32_t foundType = physDevice.findMemoryTypeIndex(memoryFlags, memReq.memoryTypeBits);

    // Allocate memory.
    VkMemoryAllocateInfo memAllocInfo{}; FILL_S_TYPE(memAllocInfo);
    memAllocInfo.allocationSize = memReq.size;
    memAllocInfo.memoryTypeIndex = foundType;

    const VkResult allocRes = vkAllocateMemory(_dev, &memAllocInfo, nullptr, &_devMem);
    setHasError(allocRes != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkAllocateMemory returned "s + getVkResultString(allocRes));
    }
}

void Buffer::bindMemory(const VkDeviceSize offset) noexcept {
    assert(_dev != VK_NULL_HANDLE && _buf != VK_NULL_HANDLE && _devMem != VK_NULL_HANDLE);

    const VkResult bindBufResult = vkBindBufferMemory(_dev, _buf, _devMem, offset);
    setHasError(bindBufResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkBindBuffer returned "s + getVkResultString(bindBufResult));
    }
}

void Buffer::copyToImage(Image &image, const uint32_t width, const uint32_t height, CommandBuffer &commandBuffer) {
    VkBufferImageCopy region{};

    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer.getHandle(), _buf, image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Buffer::fill(const void * const dataToCopy, const VkDeviceSize dataSize, const size_t offset) {
    assert(_dev != VK_NULL_HANDLE && _devMem != VK_NULL_HANDLE);

    void *data = nullptr;
    const VkResult mapRes = vkMapMemory(_dev, _devMem, offset, dataSize, 0, &data);
    setHasError(mapRes != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkMapMemory returned "s + getVkResultString(mapRes));
        return;
    }

    memcpy(data, dataToCopy, dataSize);
    vkUnmapMemory(_dev, _devMem);
}

VkDeviceSize Buffer::getSize() const noexcept {
    return _bufSize;
}

}

