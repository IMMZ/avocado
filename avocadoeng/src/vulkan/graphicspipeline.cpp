#include "graphicspipeline.hpp"

#include "vkutils.hpp"

#include "states/colorblendstate.hpp"
#include "states/dynamicstate.hpp"
#include "states/inputasmstate.hpp"
#include "states/multisamplestate.hpp"
#include "states/rasterizationstate.hpp"
#include "states/vertexinputstate.hpp"
#include "states/viewportstate.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cassert>

using namespace std::string_literals;

namespace avocado::vulkan {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(VkDevice device):
    _device(device) {
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

// todo must create multiple pipelines.
// vkCreateGraphicsPipeline(s)
GraphicsPipelineBuilder::PipelineUniquePtr GraphicsPipelineBuilder::buildPipeline(const std::vector<VkPipelineShaderStageCreateInfo> &shaderStageCIs, VkRenderPass renderPass) {
    auto pipelineCI = createStruct<VkGraphicsPipelineCreateInfo>();

    pipelineCI.stageCount = static_cast<uint32_t>(shaderStageCIs.size());
    pipelineCI.pStages = shaderStageCIs.data();

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

    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


    auto pipelineDestroyer = std::bind(vkDestroyPipeline, _device, std::placeholders::_1, nullptr);

    auto createInfo = createStruct<VkDescriptorSetLayoutCreateInfo>();
    createInfo.bindingCount = 1;
    createInfo.pBindings = &layoutBinding;

    const VkResult result1 = vkCreateDescriptorSetLayout(_device, &createInfo, nullptr, &_descriptorSetLayout);
    setHasError(result1 != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateDescriptorSetLayout returned "s + getVkResultString(result1));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }


    auto pipelineLayoutCreateInfo = createStruct<VkPipelineLayoutCreateInfo>();

    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
    const VkResult pipelineCreationResult = vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout);
    setHasError(pipelineCreationResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreatePipelineLayout returned "s + getVkResultString(pipelineCreationResult));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }

    pipelineCI.layout = _pipelineLayout;
    pipelineCI.renderPass = renderPass;

    // Create the pipeline.
    VkPipeline pipeline = VK_NULL_HANDLE;
    const VkResult result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateGraphicsPipelines returned "s + getVkResultString(result));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }

    return PipelineUniquePtr(pipeline, pipelineDestroyer);
}

void GraphicsPipelineBuilder::destroyPipeline() {
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
}

} // namespace avocado::vulkan.

