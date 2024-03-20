#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "pointertypes.hpp"

#include "../errorstorage.hpp"

namespace avocado::vulkan
{

class LogicalDevice;
class PhysicalDevice;

class Image: public core::ErrorStorage {
public:
    explicit Image(LogicalDevice &device, const uint32_t width, const uint32_t height, const VkImageType imageType);
    void allocateMemory(PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags);
    void bindMemory();
    void create();
    VkImage getHandle() noexcept;
    void setArrayLayerCount(const uint32_t count);
    void setDepth(const uint32_t depth);
    void setFormat(const VkFormat format);
    void setMipLevels(const uint32_t mipLevels);
    void setImageTiling(const VkImageTiling tiling);
    void setSampleCount(const VkSampleCountFlagBits sampleCountFlags);
    void setSharingMode(const VkSharingMode sharingMode);
    void setUsage(const VkImageUsageFlags usageFlags);

private:
    ImagePtr _handle;
    DeviceMemoryPtr _textureImageMemory;
    VkImageCreateInfo _createInfo;
    LogicalDevice &_device;
};

} // namespace avocado::vulkan.

#endif // IMAGE_HPP

