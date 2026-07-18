#include "sampler.hpp"

#include "../vulkan/structuretypes.hpp"

namespace avocado::core {

VkSamplerCreateInfo Sampler::generateVulkanCreateInfo() const {
    DEFINE_VK_STRUCTURE(VkSamplerCreateInfo, createInfo);
    createInfo.minFilter = _minFilter;
    createInfo.magFilter = _magFilter;
    createInfo.addressModeU = static_cast<VkSamplerAddressMode>(_wrapS);
    createInfo.addressModeV = static_cast<VkSamplerAddressMode>(_wrapT);
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.anisotropyEnable = VK_FALSE;
    // todo Implement!
    //VkPhysicalDeviceProperties properties{}; vkGetPhysicalDeviceProperties(physicalDevice.getHandle(), &properties);
    //samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.mipLodBias = 0.0f;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;

    return createInfo;
}

} // namespace avocado::core
