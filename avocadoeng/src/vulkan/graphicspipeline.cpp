#include "graphicspipeline.hpp"

#include "vkutils.hpp"

#include "states/colorblendstate.hpp"
#include "states/dynamicstate.hpp"
#include "states/inputasmstate.hpp"
#include "states/multisamplestate.hpp"
#include "states/rasterizationstate.hpp"
#include "states/vertexinputstate.hpp"
#include "states/viewportstate.hpp"

using namespace std::string_literals;

namespace avocado::vulkan {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(LogicalDevice &device):
    _logicalDevice(device),
    _pipelineLayout(device.createObjectPointer<VkPipelineLayout>(VK_NULL_HANDLE)) {
}

VkPipelineLayout GraphicsPipelineBuilder::getPipelineLayout() noexcept {
    return _pipelineLayout.get();
}

VkDescriptorSetLayout GraphicsPipelineBuilder::getDescriptorSetLayout() noexcept {
    return _descriptorSetLayout;
}

void GraphicsPipelineBuilder::setColorBlendState(ColorBlendState &state) noexcept {
    _colorBlendState = &state;
}

void GraphicsPipelineBuilder::setDynamicState(DynamicState &dynState) noexcept {
    _dynamicState = &dynState;
}

void GraphicsPipelineBuilder::setInputAsmState(InputAsmState &inAsmState) noexcept {
    _inputAsmState = &inAsmState;
}

void GraphicsPipelineBuilder::setMultisampleState(MultisampleState &mss) noexcept {
    _multisampleState = &mss;
}

void GraphicsPipelineBuilder::setRasterizationState(RasterizationState &rastState) noexcept {
    _rasterizationState = &rastState;
}

void GraphicsPipelineBuilder::setVertexInputState(VertexInputState &vertexInState) noexcept {
    _vertexInputState = &vertexInState;
}

void GraphicsPipelineBuilder::setViewportState(ViewportState &viewportState) noexcept {
    _viewportState = &viewportState;
}

void GraphicsPipelineBuilder::addFragmentShaderModules(const std::vector<std::vector<char>> &shaderModules) {
    for (std::vector<char> shaderModule: shaderModules)
        _shaderModuleCIs.emplace_back(addShaderModule(shaderModule, VK_SHADER_STAGE_FRAGMENT_BIT));
}

void GraphicsPipelineBuilder::addVertexShaderModules(const std::vector<std::vector<char>> &shaderModules) {
    for (std::vector<char> shaderModule: shaderModules)
        _shaderModuleCIs.emplace_back(addShaderModule(shaderModule, VK_SHADER_STAGE_VERTEX_BIT));
}

void GraphicsPipelineBuilder::setDescriptorSetLayout(VkDescriptorSetLayout dstl) {
    _descriptorSetLayout = dstl;
}

VkPipelineShaderStageCreateInfo GraphicsPipelineBuilder::addShaderModule(const std::vector<char> &data, const VkShaderStageFlagBits shType) {
    auto shaderStageCreateInfo = createStruct<VkPipelineShaderStageCreateInfo>();

    auto shaderModuleCreateInfo = createStruct<VkShaderModuleCreateInfo>();
    shaderModuleCreateInfo.codeSize = data.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    const VkResult result = vkCreateShaderModule(_logicalDevice.getHandle(), &shaderModuleCreateInfo, nullptr, &shaderModule);

    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateShaderModule returned "s + getVkResultString(result));
        return shaderStageCreateInfo;
    }

    _shaderModules.emplace_back(shaderModule,
        std::bind(vkDestroyShaderModule, _logicalDevice.getHandle(), std::placeholders::_1, nullptr));

    shaderStageCreateInfo.stage = shType;
    shaderStageCreateInfo.module = _shaderModules.back().get();
    shaderStageCreateInfo.pName = "main"; // Entry point.
    return shaderStageCreateInfo;
}

// todo must create multiple pipelines.
// vkCreateGraphicsPipeline(s)
GraphicsPipelineBuilder::PipelineUniquePtr GraphicsPipelineBuilder::buildPipeline(VkRenderPass renderPass) {
    auto pipelineCI = createStruct<VkGraphicsPipelineCreateInfo>();

    pipelineCI.stageCount = static_cast<uint32_t>(_shaderModuleCIs.size());
    pipelineCI.pStages = _shaderModuleCIs.data();

    VkPipelineDynamicStateCreateInfo dynamicStateCI{};
    dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    if (_dynamicState != nullptr) {
        dynamicStateCI = createStateCreateInfo(*_dynamicState);
        pipelineCI.pDynamicState = &dynamicStateCI;
    }

    VkPipelineVertexInputStateCreateInfo vertexInStateCI{};
    vertexInStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (_vertexInputState != nullptr) {
        vertexInStateCI = createStateCreateInfo(*_vertexInputState);
        pipelineCI.pVertexInputState = &vertexInStateCI;
    }

    VkPipelineInputAssemblyStateCreateInfo inputAsmStateCI{};
    inputAsmStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    if (_inputAsmState != nullptr) {
        inputAsmStateCI = createStateCreateInfo(*_inputAsmState);
        pipelineCI.pInputAssemblyState = &inputAsmStateCI;
    }

    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    if (_viewportState != nullptr) {
        viewportStateCI = createStateCreateInfo(*_viewportState);
        pipelineCI.pViewportState = &viewportStateCI;
    }

    VkPipelineRasterizationStateCreateInfo rasterizationCI{};
    rasterizationCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    if (_rasterizationState != nullptr) {
        rasterizationCI = createStateCreateInfo(*_rasterizationState);
        pipelineCI.pRasterizationState = &rasterizationCI;
    }

    VkPipelineMultisampleStateCreateInfo multisamplingCI{};
    multisamplingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    if (_multisampleState != nullptr) {
        multisamplingCI = createStateCreateInfo(*_multisampleState);
        pipelineCI.pMultisampleState = &multisamplingCI;
    }

    VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
    colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    if (_colorBlendState != nullptr) {
        colorBlendStateCI = createStateCreateInfo(*_colorBlendState);
        pipelineCI.pColorBlendState = &colorBlendStateCI;
    }

    auto pipelineDestroyer = std::bind(vkDestroyPipeline, _logicalDevice.getHandle(), std::placeholders::_1, nullptr);

    auto pipelineLayoutCreateInfo = createStruct<VkPipelineLayoutCreateInfo>();

    pipelineLayoutCreateInfo.setLayoutCount = _dsLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = _dsLayouts.data();

    VkPipelineLayout pipelineLayout;
    const VkResult pipelineCreationResult = vkCreatePipelineLayout(_logicalDevice.getHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    setHasError(pipelineCreationResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreatePipelineLayout returned "s + getVkResultString(pipelineCreationResult));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }

    _pipelineLayout.reset(pipelineLayout);

    pipelineCI.layout = _pipelineLayout.get();
    pipelineCI.renderPass = renderPass;

    // Create the pipeline.
    VkPipeline pipeline = VK_NULL_HANDLE;
    const VkResult result = vkCreateGraphicsPipelines(_logicalDevice.getHandle(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);

    // Free unneeded resources.
    _shaderModuleCIs.clear();
    _shaderModuleCIs.shrink_to_fit();

    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateGraphicsPipelines returned "s + getVkResultString(result));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }

    return PipelineUniquePtr(pipeline, pipelineDestroyer);
}

void GraphicsPipelineBuilder::setDSLayouts(std::vector<VkDescriptorSetLayout> &layouts)
{
    _dsLayouts = layouts;
}

void GraphicsPipelineBuilder::destroyPipeline() {
    vkDestroyDescriptorSetLayout(_logicalDevice.getHandle(), _descriptorSetLayout, nullptr);
}

} // namespace avocado::vulkan.

