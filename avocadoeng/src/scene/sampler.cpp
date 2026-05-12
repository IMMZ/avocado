#include "sampler.hpp"

#include "../vulkan/structuretypes.hpp"

namespace avocado::core {

VkSamplerCreateInfo Sampler::generateVulkanCreateInfo() const {
    DEFINE_VK_STRUCTURE(VkSamplerCreateInfo, createInfo);
    createInfo.minFilter = _minFilter;
    createInfo.magFilter = _magFilter;
    createInfo.addressModeU = static_cast<VkSamplerAddressMode>(_wrapS);
    createInfo.addressModeV = static_cast<VkSamplerAddressMode>(_wrapT);
    return createInfo;
}

} // namespace avocado::core
