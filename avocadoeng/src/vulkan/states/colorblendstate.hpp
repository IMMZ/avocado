#ifndef AVOCADO_VULKAN_COLOR_BLEND_STATE
#define AVOCADO_VULKAN_COLOR_BLEND_STATE

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class ColorBlendState {
public:
    VkBool32 isLogicOpEnabled() const noexcept;
    void setLogicOpEnabled(const VkBool32 enabled);
    VkLogicOp getLogicOp() const noexcept;
    void setLogicOp(const VkLogicOp logicOp);
    uint32_t getAttachmentCount() const noexcept;
    void addAttachment(const VkPipelineColorBlendAttachmentState &attachment);
    void addAttachment(VkPipelineColorBlendAttachmentState &&attachment);
    VkPipelineColorBlendAttachmentState* getAttachments();

    VkPipelineColorBlendStateCreateInfo createCreateInfo();

private:
    std::vector<VkPipelineColorBlendAttachmentState> _attachments;
    VkBool32 _logicOpEnable = VK_FALSE;
    VkLogicOp _logicOp = VK_LOGIC_OP_MAX_ENUM;
};

}

#endif

