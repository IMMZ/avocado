#include "image.hpp"

#include "logicaldevice.hpp"
#include "physicaldevice.hpp"
#include "structuretypes.hpp"

namespace avocado::vulkan
{

Image::Image(LogicalDevice &device, const uint32_t width, const uint32_t height, const VkImageType imageType):
    _handle(device.createObjectPointer<VkImage>(VK_NULL_HANDLE)),
    _textureImageMemory(device.createAllocatedObjectPointer<VkDeviceMemory>(VK_NULL_HANDLE)),
    _createInfo{},
    _device(device) {
    FILL_S_TYPE(_createInfo);
    _createInfo.extent.width = width;
    _createInfo.extent.height = height;
    _createInfo.imageType = imageType;
    _createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void Image::allocateMemory(PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags) {
    // todo Same code with Buffer class. Extract somewhere.
    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(_device.getHandle(), _handle.get(), &memRequirements);

    const uint32_t foundType = physDevice.findMemoryTypeIndex(memoryFlags, memRequirements.memoryTypeBits);

    VkMemoryAllocateInfo imageMemAI{}; FILL_S_TYPE(imageMemAI);
    imageMemAI.allocationSize = memRequirements.size;
    imageMemAI.memoryTypeIndex = foundType;

    VkDeviceMemory deviceMemory;
    const VkResult allocationResult = vkAllocateMemory(_device.getHandle(), &imageMemAI, nullptr, &deviceMemory);
    setHasError(allocationResult != VK_SUCCESS);
    if (!hasError())
        _textureImageMemory.reset(deviceMemory);
    else
        setErrorMessage("vkAllocateMemory returned "s + getVkResultString(allocationResult));
}

void Image::bindMemory() {
    const VkResult bindResult = vkBindImageMemory(_device.getHandle(), _handle.get(), _textureImageMemory.get(), 0);
    setHasError(bindResult != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkBindImageMemory returned "s + getVkResultString(bindResult));
}

void Image::create() {
    if (_handle != VK_NULL_HANDLE)
        return;

    VkImage imageToCreate;
    VkResult result = vkCreateImage(_device.getHandle(), &_createInfo, nullptr, &imageToCreate);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateImage returned "s );
        return;
    }

    _handle.reset(imageToCreate);
}

VkImage Image::getHandle() noexcept {
    return _handle.get();
}

void Image::setArrayLayerCount(const uint32_t count) {
    _createInfo.arrayLayers = count;
}

void Image::setDepth(const uint32_t depth) {
    _createInfo.extent.depth = depth;
}

void Image::setFormat(const VkFormat format) {
    _createInfo.format = format;
}

void Image::setMipLevels(const uint32_t mipLevels) {
    _createInfo.mipLevels = mipLevels;
}

void Image::setImageTiling(const VkImageTiling tiling) {
    _createInfo.tiling = tiling;
}

void Image::setSampleCount(const VkSampleCountFlagBits sampleCountFlags) {
    _createInfo.samples = sampleCountFlags;
}

void Image::setSharingMode(const VkSharingMode sharingMode) {
    _createInfo.sharingMode = sharingMode;
}

void Image::setUsage(const VkImageUsageFlags usageFlags) {
    _createInfo.usage = usageFlags;
}

} // namespace avocado::vulkan.

