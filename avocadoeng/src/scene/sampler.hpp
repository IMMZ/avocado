#ifndef SCENE_SAMPLER_HPP
#define SCENE_SAMPLER_HPP

#include <vulkan/vulkan_core.h>

namespace avocado::core {

struct Sampler {
    VkFilter _minFilter = VK_FILTER_MAX_ENUM;
    VkFilter _magFilter = VK_FILTER_MAX_ENUM;

    enum class TextureWrap {
        Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT
        , MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
        , ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    };
    TextureWrap _wrapS = TextureWrap::Repeat;
    TextureWrap _wrapT = TextureWrap::Repeat;

    VkSamplerCreateInfo generateVulkanCreateInfo() const;
};

} // namespace avocado::core

#endif /* ifndef SCENE_HPP */
