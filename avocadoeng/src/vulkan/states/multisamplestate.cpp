#include "multisamplestate.hpp"

#include "../vkutils.hpp"

namespace avocado::vulkan {

float MultisampleState::getMinSampleShading() const noexcept {
    return _minSampleShading;
}

void MultisampleState::setMinSampleShading(const float minSampleShading) {
    _minSampleShading = minSampleShading;
}

VkSampleCountFlagBits MultisampleState::getRasterizationSamples() const noexcept {
    return _rasterizationSamples;
}

void MultisampleState::setRasterizationSamples(const VkSampleCountFlagBits rastSamples) noexcept {
    _rasterizationSamples = rastSamples;
}

VkPipelineMultisampleStateCreateInfo MultisampleState::createCreateInfo() noexcept {
    auto multisamplingCI = createStruct<VkPipelineMultisampleStateCreateInfo>();
    multisamplingCI.rasterizationSamples = getRasterizationSamples();
    multisamplingCI.minSampleShading = getMinSampleShading();
    return multisamplingCI;
}

}

