#include "image.hpp"

#include "logicaldevice.hpp"
#include "physicaldevice.hpp"
#include "structuretypes.hpp"

namespace avocado::vulkan
{

Image::Image(LogicalDevice &device, const uint32_t imgWidth, const uint32_t imgHeight, const VkImageType imageType):
    _handle(device.createObjectPointer<VkImage>(VK_NULL_HANDLE)),
    _textureImageMemory(device.createAllocatedObjectPointer<VkDeviceMemory>(VK_NULL_HANDLE)),
    _createInfo{},
    _device(device) {
    _createInfo.sType = StructureType<decltype(_createInfo)>;
    _createInfo.extent.width = imgWidth;
    _createInfo.extent.height = imgHeight;
    _createInfo.imageType = imageType;
    _createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    width = imgWidth;
    height = imgHeight;
}

void Image::allocateMemory(PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags) {
    // todo Same code with Buffer class. Extract somewhere.
    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(_device.getHandle(), _handle.get(), &memRequirements);

    const uint32_t foundType = physDevice.findMemoryTypeIndex(memoryFlags, memRequirements.memoryTypeBits);

    DEFINE_VK_STRUCTURE(VkMemoryAllocateInfo, imageMemAI);
    imageMemAI.allocationSize = memRequirements.size;
    imageMemAI.memoryTypeIndex = foundType;

    VkDeviceMemory deviceMemory;
    CALL_VULKAN(vkAllocateMemory, _device.getHandle(), &imageMemAI, nullptr, &deviceMemory);
    if (VK_SUCCESS == callResult)
        _textureImageMemory.reset(deviceMemory);
}

void Image::bindMemory() {
    CALL_VULKAN(vkBindImageMemory, _device.getHandle(), _handle.get(), _textureImageMemory.get(), 0);
}

void Image::create() {
    if (_handle != VK_NULL_HANDLE)
        return;

    VkImage imageToCreate;
    CALL_VULKAN_AND_RETURN(vkCreateImage, _device.getHandle(), &_createInfo, nullptr, &imageToCreate);
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

