#include "vertexinputstate.hpp"

namespace avocado::vulkan {

void VertexInputState::addBindingDescription(const uint32_t binding, const uint32_t stride, VkVertexInputRate inRate) {
    _bindingDescriptions.push_back({binding, stride, inRate});
}

void VertexInputState::addAttributeDescription(const uint32_t loc, const uint32_t binding, const VkFormat format, const uint32_t offset) {
    _attributeDescriptions.push_back({loc, binding, format, offset});

}

VkVertexInputBindingDescription* VertexInputState::getBindingDescriptionData() {
    return _bindingDescriptions.data();
}

VkVertexInputAttributeDescription* VertexInputState::getAttributeDescriptionData() {
    return _attributeDescriptions.data();
}

uint32_t VertexInputState::getBindingDescriptionsCount() const {
    return static_cast<uint32_t>(_bindingDescriptions.size());
}

uint32_t VertexInputState::getAttributeDescriptionsCount() const {
    return static_cast<uint32_t>(_attributeDescriptions.size());
}

}

