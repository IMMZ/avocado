#include "dynamicstate.hpp"

#include "../vkutils.hpp"

namespace avocado::vulkan {
    
DynamicState::DynamicState(const std::vector<VkDynamicState> &dynStates):
    _dynamicStates(dynStates) {
}

DynamicState::DynamicState(std::vector<VkDynamicState> &&dynStates):
    _dynamicStates(std::move(dynStates)) {
}

uint32_t DynamicState::getDynamicStateCount() const noexcept {
    return static_cast<uint32_t>(_dynamicStates.size());
}

const VkDynamicState* DynamicState::getDynamicStates() noexcept {
    return _dynamicStates.data();
}

void DynamicState::setDynamicStates(const std::vector<VkDynamicState> &dynStates) {
    _dynamicStates = dynStates;
}

void DynamicState::setDynamicStates(std::vector<VkDynamicState> &&dynStates) noexcept {
    _dynamicStates = std::move(dynStates);
}

VkPipelineDynamicStateCreateInfo DynamicState::createCreateInfo() noexcept {
    auto dynamicStateCI = createStruct<VkPipelineDynamicStateCreateInfo>();
    dynamicStateCI.dynamicStateCount = getDynamicStateCount();
    dynamicStateCI.pDynamicStates = getDynamicStates();
    return dynamicStateCI;
}

}

