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

    if (_dynamicState != nullptr) {
        const VkPipelineDynamicStateCreateInfo &dynamicStateCI = createStateCreateInfo(*_dynamicState);
        pipelineCI.pDynamicState = &dynamicStateCI;
    }

    if (_vertexInputState != nullptr) {
        const VkPipelineVertexInputStateCreateInfo &vertexInStateCI = createStateCreateInfo(*_vertexInputState);
        pipelineCI.pVertexInputState = &vertexInStateCI;
    }

    if (_inputAsmState != nullptr) {
        const VkPipelineInputAssemblyStateCreateInfo &inputAsmStateCI = createStateCreateInfo(*_inputAsmState);
        pipelineCI.pInputAssemblyState = &inputAsmStateCI;
    }

    if (_viewportState != nullptr) {
        const VkPipelineViewportStateCreateInfo &viewportStateCI = createStateCreateInfo(*_viewportState);
        pipelineCI.pViewportState = &viewportStateCI;
    }

    if (_rasterizationState != nullptr) {
        const VkPipelineRasterizationStateCreateInfo &rasterizationCI = createStateCreateInfo(*_rasterizationState);
        pipelineCI.pRasterizationState = &rasterizationCI;
    }

    if (_multisampleState != nullptr) {
        const VkPipelineMultisampleStateCreateInfo &multisamplingCI = createStateCreateInfo(*_multisampleState);
        pipelineCI.pMultisampleState = &multisamplingCI;
    }

    if (_colorBlendState != nullptr) {
        const VkPipelineColorBlendStateCreateInfo &colorBlendStateCI = createStateCreateInfo(*_colorBlendState);
        pipelineCI.pColorBlendState = &colorBlendStateCI;
    }

    // Pipeline layout.
    using PipelineLayoutDestroyer = decltype(std::bind(vkDestroyPipelineLayout, _device, std::placeholders::_1, nullptr));
    using PipelineLayoutUniquePtr =
        std::unique_ptr<std::remove_pointer_t<VkPipelineLayout>, PipelineLayoutDestroyer>;

    auto pipelineLayoutCreateInfo = createStruct<VkPipelineLayoutCreateInfo>();
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    const VkResult pipelineCreationResult = vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout);
    auto pipelineDestroyer = std::bind(vkDestroyPipeline, _device, std::placeholders::_1, nullptr);
    setHasError(pipelineCreationResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreatePipelineLayout returned "s + getVkResultString(pipelineCreationResult));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }

    PipelineLayoutUniquePtr pipelineLayout(_pipelineLayout, std::bind(vkDestroyPipelineLayout, _device, std::placeholders::_1, nullptr));
    pipelineCI.layout = pipelineLayout.get();
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

} // namespace avocado::vulkan.

