#include "image.hpp"

#include "logicaldevice.hpp"


namespace avocado::vulkan
{

Image::Image(LogicalDevice &device, const uint32_t width, const uint32_t height, const VkImageType imageType):
    _handle(device.createObjectPointer<VkImage>(VK_NULL_HANDLE)),
    _createInfo{},
    _device(device) {
    _createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _createInfo.extent.width = width;
    _createInfo.extent.height = height;
    _createInfo.imageType = imageType;
    _createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

VkImage Image::getHandle() {
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

