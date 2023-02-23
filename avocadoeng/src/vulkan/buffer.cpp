#include "buffer.hpp"
#include "logicaldevice.hpp"
#include "physicaldevice.hpp"

#include "vkutils.hpp"

using namespace std::string_literals;

namespace avocado::vulkan {

Buffer::Buffer(const VkDeviceSize size, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, LogicalDevice &logicalDevice,
    PhysicalDevice &physDevice, const std::vector<QueueFamily> &queueFamilies):
    _dev(logicalDevice.getHandle()),
    _physDev(physDevice.getHandle()),
    _bufSize(size) {
    auto bufferCI = avocado::vulkan::createStruct<VkBufferCreateInfo>();
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
    , _physDev(std::move(other._physDev))
    , _devMem(std::move(other._devMem))
    , _bufSize(std::move(other._bufSize)) {
    other._dev = VK_NULL_HANDLE;
    other._buf = VK_NULL_HANDLE;
    other._physDev = VK_NULL_HANDLE;
    other._devMem = VK_NULL_HANDLE;
    other._bufSize = 0;
}

Buffer& Buffer::operator=(Buffer &&other) {
    core::ErrorStorage::operator=(other);

    _dev = std::move(other._dev);
    _buf = std::move(other._buf);
    _physDev = std::move(other._physDev);
    _devMem = std::move(other._devMem);
    _bufSize = std::move(other._bufSize);

    other._dev = VK_NULL_HANDLE;
    other._buf = VK_NULL_HANDLE;
    other._physDev = VK_NULL_HANDLE;
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

void Buffer::allocateMemory(const VkMemoryPropertyFlags memoryFlags) noexcept {
    assert(_buf != VK_NULL_HANDLE && _dev != VK_NULL_HANDLE && _physDev != VK_NULL_HANDLE);
    assert(_devMem == VK_NULL_HANDLE);

    // Memory requirements.
    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(_dev, _buf, &memReq);

    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(_physDev, &memProps);

    // Find needed type of memory by typeFilter & properties.
    const uint32_t typeFilter = memReq.memoryTypeBits;

    uint32_t foundType = 0;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags) {
            foundType = i;
            break;
        }
    }

    // Allocate memory.
    VkMemoryAllocateInfo memAllocInfo = avocado::vulkan::createStruct<VkMemoryAllocateInfo>();
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

void Buffer::fill(const void * const dataToCopy, const VkDeviceSize dataSize, const size_t offset) {
    assert(_dev != VK_NULL_HANDLE && _devMem != VK_NULL_HANDLE);

    void *data = nullptr;
    const VkResult mapRes = vkMapMemory(_dev, _devMem, offset, _bufSize, 0, &data);
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

void Buffer::copyDataToBuffer(Buffer &dstBuf) const {
    auto cmdBufAllocInfo = createStruct<VkCommandBufferAllocateInfo>();
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}

}

