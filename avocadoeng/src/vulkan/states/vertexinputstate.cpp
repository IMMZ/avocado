#include "vertexinputstate.hpp"

#include "../vkutils.hpp"

namespace avocado::vulkan {

void VertexInputState::addBindingDescription(const uint32_t binding, const uint32_t stride, VkVertexInputRate inRate) {
    _bindingDescriptions.push_back({binding, stride, inRate});
}

void VertexInputState::addAttributeDescription(const uint32_t loc, const uint32_t binding, const VkFormat format, const uint32_t offset) {
    _attributeDescriptions.push_back({loc, binding, format, offset});

}

VkVertexInputBindingDescription* VertexInputState::getBindingDescriptionData() noexcept {
    return _bindingDescriptions.data();
}

VkVertexInputAttributeDescription* VertexInputState::getAttributeDescriptionData() noexcept {
    return _attributeDescriptions.data();
}

uint32_t VertexInputState::getBindingDescriptionsCount() const noexcept {
    return static_cast<uint32_t>(_bindingDescriptions.size());
}

uint32_t VertexInputState::getAttributeDescriptionsCount() const noexcept {
    return static_cast<uint32_t>(_attributeDescriptions.size());
}

VkPipelineVertexInputStateCreateInfo VertexInputState::createCreateInfo() noexcept {
    auto vertexInStateCI = createStruct<VkPipelineVertexInputStateCreateInfo>();
    vertexInStateCI.pVertexAttributeDescriptions = getAttributeDescriptionData();
    vertexInStateCI.vertexAttributeDescriptionCount = getAttributeDescriptionsCount();
    vertexInStateCI.pVertexBindingDescriptions = getBindingDescriptionData();
    vertexInStateCI.vertexBindingDescriptionCount = getBindingDescriptionsCount();
    return vertexInStateCI;
}

}

