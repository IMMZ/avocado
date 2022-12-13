#include "colorblendstate.hpp"

#include "../vkutils.hpp"

namespace avocado::vulkan {

VkBool32 ColorBlendState::isLogicOpEnabled() const noexcept {
    return _logicOpEnable;
}

void ColorBlendState::setLogicOpEnabled(const VkBool32 enabled) {
    _logicOpEnable = enabled;
}

VkLogicOp ColorBlendState::getLogicOp() const noexcept {
    return _logicOp;
}

void ColorBlendState::setLogicOp(const VkLogicOp logicOp) {
    _logicOp = logicOp;
}

uint32_t ColorBlendState::getAttachmentCount() const noexcept {
    return static_cast<uint32_t>(_attachments.size());
}

void ColorBlendState::addAttachment(const VkPipelineColorBlendAttachmentState &attachment) {
    _attachments.push_back(attachment);
}

void ColorBlendState::addAttachment(VkPipelineColorBlendAttachmentState &&attachment) {
    _attachments.emplace_back(std::move(attachment));
}

VkPipelineColorBlendAttachmentState* ColorBlendState::getAttachments() {
    return _attachments.data();
}

VkPipelineColorBlendStateCreateInfo ColorBlendState::createCreateInfo() {
    auto colorBlendStateCI = createStruct<VkPipelineColorBlendStateCreateInfo>();
    colorBlendStateCI.logicOpEnable = isLogicOpEnabled();
    colorBlendStateCI.logicOp = getLogicOp();
    colorBlendStateCI.attachmentCount = getAttachmentCount();
    colorBlendStateCI.pAttachments = getAttachments();
    return colorBlendStateCI;
}

}

