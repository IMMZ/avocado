#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "pointertypes.hpp"

#include "../errorstorage.hpp"

namespace avocado::vulkan
{

class LogicalDevice;

class Image: public core::ErrorStorage {
public:
    explicit Image(LogicalDevice &device, const uint32_t width, const uint32_t height, const VkImageType imageType);
    void create();
    VkImage getHandle();
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
    VkImageCreateInfo _createInfo;
    LogicalDevice &_device;
};

} // namespace avocado::vulkan.

#endif // IMAGE_HPP

