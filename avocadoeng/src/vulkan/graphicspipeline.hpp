#ifndef AVOCADO_VULKAN_GRAPHICS_PIPELINE
#define AVOCADO_VULKAN_GRAPHICS_PIPELINE

#include "../errorstorage.hpp"

#include "logicaldevice.hpp"
#include "pointertypes.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace avocado::vulkan {

class GraphicsPipelineBuilder: public avocado::core::ErrorStorage {
public:
    explicit GraphicsPipelineBuilder(LogicalDevice &device);

    PipelinePtr createPipeline(VkRenderPass renderPass);
    VkPipelineLayout getPipelineLayout() noexcept;
    void addColorBlendAttachment(const VkPipelineColorBlendAttachmentState &state);
    void addColorBlendAttachment(VkPipelineColorBlendAttachmentState &&state);
    void setDynamicStates(const std::vector<VkDynamicState> &dynStates);
    void setDynamicStates(std::vector<VkDynamicState> &&dynStates);
    void addBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inRate);
    void addAttributeDescription(const uint32_t loc, const uint32_t binding, const VkFormat format, const uint32_t offset);
    VkPipelineColorBlendStateCreateInfo& createColorBlendState();
    VkPipelineDepthStencilStateCreateInfo& createDepthStencilState();
    VkPipelineDynamicStateCreateInfo& createDynamicState();
    VkPipelineInputAssemblyStateCreateInfo& createInputAssmeblyState();
    VkPipelineMultisampleStateCreateInfo& createMultisampleState();
    VkPipelineRasterizationStateCreateInfo& createRasterizationState();
    VkPipelineVertexInputStateCreateInfo& createVertexInputState();
    VkPipelineViewportStateCreateInfo& createViewportState();
    void setViewPorts(const std::vector<VkViewport> &viewports);
    void setViewPorts(std::vector<VkViewport> &&viewports);
    void setScissors(const std::vector<VkRect2D> &scissors);
    void setScissors(std::vector<VkRect2D> &&scissors);
    void addFragmentShaderModules(const std::vector<std::vector<char>> &shaderModules);
    void addVertexShaderModules(const std::vector<std::vector<char>> &shaderModules);
    void setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> &layouts);
    void loadShaders(const std::string &shaderPath);

private:
    VkPipelineShaderStageCreateInfo addShaderModule(const std::vector<char> &data, const VkShaderStageFlagBits shType);
    void bindStages(VkGraphicsPipelineCreateInfo &pipelineCreateInfo) noexcept;
    void createLayout(VkGraphicsPipelineCreateInfo &pipelineCreateInfo) noexcept;

    std::vector<VkPipelineColorBlendAttachmentState> _colorBlendAttachments; // For color blend state.
    std::vector<VkPipelineShaderStageCreateInfo> _shaderModuleCIs;
    std::vector<ShaderModulePtr> _shaderModules;
    std::vector<VkDynamicState> _dynamicStates; // For dynamic state.

    // For vertex input state.
    std::vector<VkVertexInputBindingDescription> _bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;

    // For viewport state.
    std::vector<VkViewport> _viewports;
    std::vector<VkRect2D> _scissors;

    std::unique_ptr<VkPipelineColorBlendStateCreateInfo> _colorBlendState;
    std::unique_ptr<VkPipelineDynamicStateCreateInfo> _dynamicState;
    std::unique_ptr<VkPipelineVertexInputStateCreateInfo> _vertexInputState;
    std::unique_ptr<VkPipelineInputAssemblyStateCreateInfo> _inputAsmState;
    std::unique_ptr<VkPipelineMultisampleStateCreateInfo> _multisampleState;
    std::unique_ptr<VkPipelineRasterizationStateCreateInfo> _rasterizationState;
    std::unique_ptr<VkPipelineViewportStateCreateInfo> _viewportState;
    std::unique_ptr<VkPipelineDepthStencilStateCreateInfo> _depthStencilState;
    std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
    PipelineLayoutPtr _pipelineLayout;
    LogicalDevice &_logicalDevice;
};

} // namespace avocado::vulkan.

#endif

