#ifndef AVOCADO_VULKAN_MULTISAMPLE_STATE
#define AVOCADO_VULKAN_MULTISAMPLE_STATE

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

class MultisampleState {
public:
    float getMinSampleShading() const noexcept;
    void setMinSampleShading(const float minSampleShading);
    VkSampleCountFlagBits getRasterizationSamples() const noexcept;
    void setRasterizationSamples(const VkSampleCountFlagBits rastSamples) noexcept;

    VkPipelineMultisampleStateCreateInfo createCreateInfo() noexcept;

private:
    float _minSampleShading = 0.f;
    VkSampleCountFlagBits _rasterizationSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
};

}

#endif

