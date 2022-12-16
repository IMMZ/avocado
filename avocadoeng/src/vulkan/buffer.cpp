#include "buffer.hpp"
#include "logicaldevice.hpp"

#include "vkutils.hpp"

using namespace std::string_literals;

namespace avocado::vulkan {

Buffer::Buffer(const VkDeviceSize size, const Usage usage, const SharingMode sharingMode, LogicalDevice &logicalDevice):
    _dev(logicalDevice.getHandle()),
    _usage(usage) {
    auto bufferCI = avocado::vulkan::createStruct<VkBufferCreateInfo>();
    bufferCI.size = size;
    bufferCI.usage = static_cast<decltype(bufferCI.usage)>(_usage);
    bufferCI.sharingMode = static_cast<decltype(bufferCI.sharingMode)>(sharingMode);

    const VkResult result = vkCreateBuffer(_dev, &bufferCI, nullptr, &_buf);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkCreateBuffer returned "s + getVkResultString(result));
}

Buffer::Buffer(Buffer &&other):
    _dev(std::move(other._dev))
    , _buf(std::move(other._buf))
    , _usage(std::move(other._usage)) {
    other._dev = VK_NULL_HANDLE;
    other._buf = VK_NULL_HANDLE;
}

Buffer& Buffer::operator=(Buffer &&other) {
    core::ErrorStorage::operator=(other);

    _dev = std::move(other._dev);
    _buf = std::move(other._buf);
    _usage = std::move(other._usage);
    other._dev = VK_NULL_HANDLE;
    other._buf = VK_NULL_HANDLE;
    return *this;
}

Buffer::~Buffer() {
    if (_buf != VK_NULL_HANDLE)
        vkDestroyBuffer(_dev, _buf, nullptr);
}

VkBuffer Buffer::getHandle() noexcept {
    return _buf;
}

Buffer::Usage Buffer::getUsage() const noexcept {
    return _usage;
}

}

