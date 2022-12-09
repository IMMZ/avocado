#include "graphicspipeline.hpp"

#include "vertexinputstate.hpp"
#include "vkutils.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cassert>

using namespace std::string_literals;

namespace avocado::vulkan {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(VkDevice device):
    _device(device) {
}

void GraphicsPipelineBuilder::addDynamicStates(const std::initializer_list<DynamicState> &dynStates) {
    for (const DynamicState ds: dynStates) {
        _dynamicStates.push_back(static_cast<VkDynamicState>(ds));
    }
}

void GraphicsPipelineBuilder::addDynamicState(DynamicState dynState) {
    _dynamicStates.push_back(static_cast<VkDynamicState>(dynState));
}

void GraphicsPipelineBuilder::setPrimitiveTopology(const PrimitiveTopology primTop) {
    _primitiveTopology = primTop;
}

void GraphicsPipelineBuilder::setPolygonMode(const PolygonMode polyMode) {
    _polygonMode = polyMode;
}

void GraphicsPipelineBuilder::setCullMode(const CullMode cullMode) {
    _cullMode = cullMode;
}

void GraphicsPipelineBuilder::setFrontFace(const FrontFace ff) {
    _frontFace = ff;
}

void GraphicsPipelineBuilder::setLineWidth(const float lineW) {
    _lineWidth = lineW;
}

void GraphicsPipelineBuilder::setDepthClampEnable(const bool enable) {
    _depthClamEnable = enable;
}

void GraphicsPipelineBuilder::setRasterizerDiscardEnable(const bool enable) {
    _rasterizerDiscardEnable = enable;
}

void GraphicsPipelineBuilder::setDepthBiasEnable(const bool enable) {
    _depthBiasEnable = enable;
}

void GraphicsPipelineBuilder::setSampleCount(const SampleCount sampleCnt) {
    _sampleCount = sampleCnt;
}

void GraphicsPipelineBuilder::setMinSampleShading(const float minSampleShading) {
    _minSampleShading = minSampleShading;
}


void GraphicsPipelineBuilder::setLogicOperation(const LogicOperation lo) {
    _logicOperation = lo;
}

void GraphicsPipelineBuilder::setLogicOperationEnable(const bool enable) {
    _logicOperationEnable = enable;
}

void GraphicsPipelineBuilder::addColorAttachment(const ColorAttachment &ca) {
    _colorAttachments.push_back(ca);
}

void GraphicsPipelineBuilder::addColorAttachment(ColorAttachment &&ca) {
    _colorAttachments.emplace_back(std::move(ca));
}

void GraphicsPipelineBuilder::setVertexInputState(VertexInputState &vertexInState) {
    _vertexInputState = &vertexInState;
}

// todo Looks like heavy function. Refactor?
GraphicsPipelineBuilder::PipelineUniquePtr GraphicsPipelineBuilder::buildPipeline(const std::vector<VkPipelineShaderStageCreateInfo> &shaderStageCIs, VkFormat format, VkRenderPass renderPass, const std::vector<VkViewport> &viewports, const std::vector<VkRect2D> &scissors) {
    auto pipelineCI = createStruct<VkGraphicsPipelineCreateInfo>();

    // Shader modules.
    pipelineCI.stageCount = static_cast<uint32_t>(shaderStageCIs.size());
    pipelineCI.pStages = shaderStageCIs.data();

    // Dynamic states.
    auto dynamicStateCI = createStruct<VkPipelineDynamicStateCreateInfo>();
    dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size());
    dynamicStateCI.pDynamicStates = _dynamicStates.data();
    pipelineCI.pDynamicState = &dynamicStateCI;

    // Vertex input.
    if (_vertexInputState != nullptr) {
        auto vertexInStateCI = createStruct<VkPipelineVertexInputStateCreateInfo>();
        vertexInStateCI.pVertexAttributeDescriptions = _vertexInputState->getAttributeDescriptionData();
        vertexInStateCI.vertexAttributeDescriptionCount = _vertexInputState->getAttributeDescriptionsCount();
        vertexInStateCI.pVertexBindingDescriptions = _vertexInputState->getBindingDescriptionData();
        vertexInStateCI.vertexBindingDescriptionCount = _vertexInputState->getBindingDescriptionsCount();
        pipelineCI.pVertexInputState = &vertexInStateCI;
    }

    // Input assembly.
    auto inputAsmStateCI = createStruct<VkPipelineInputAssemblyStateCreateInfo>();
    inputAsmStateCI.topology = static_cast<VkPrimitiveTopology>(_primitiveTopology); 
    pipelineCI.pInputAssemblyState = &inputAsmStateCI;

    // Clipping.
    auto viewportStateCreateInfo = createStruct<VkPipelineViewportStateCreateInfo>();
    viewportStateCreateInfo.viewportCount = static_cast<uint32_t>(viewports.size());
    viewportStateCreateInfo.scissorCount = static_cast<uint32_t>(scissors.size());
    viewportStateCreateInfo.pViewports = viewports.data();
    viewportStateCreateInfo.pScissors = scissors.data();
    pipelineCI.pViewportState = &viewportStateCreateInfo;
    
    // Rasterizer.
    auto rasterizerCreateInfo = createStruct<VkPipelineRasterizationStateCreateInfo>();
    rasterizerCreateInfo.depthClampEnable = static_cast<VkBool32>(_depthClamEnable);
    rasterizerCreateInfo.rasterizerDiscardEnable = static_cast<VkBool32>(_rasterizerDiscardEnable);
    rasterizerCreateInfo.polygonMode = static_cast<VkPolygonMode>(_polygonMode);
    rasterizerCreateInfo.lineWidth = _lineWidth;
    rasterizerCreateInfo.cullMode = static_cast<VkFlags>(_cullMode);
    rasterizerCreateInfo.frontFace = static_cast<VkFrontFace>(_frontFace);
    rasterizerCreateInfo.depthBiasEnable = static_cast<VkBool32>(_depthBiasEnable);
    pipelineCI.pRasterizationState = &rasterizerCreateInfo;

    // Multisampling.
    auto multisamplingCI = createStruct<VkPipelineMultisampleStateCreateInfo>();
    multisamplingCI.rasterizationSamples = static_cast<VkSampleCountFlagBits>(_sampleCount);
    multisamplingCI.minSampleShading = _minSampleShading;
    pipelineCI.pMultisampleState = &multisamplingCI;

    // Color blending.
    std::vector<VkPipelineColorBlendAttachmentState> colorAttachments(_colorAttachments.size());
    assert(colorAttachments.size() == _colorAttachments.size());

    for (size_t i = 0; i < _colorAttachments.size(); ++i) {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = static_cast<VkColorComponentFlags>(_colorAttachments[i].getColorComponent());
        colorBlendAttachment.blendEnable = _colorAttachments[i].isBlendEnabled() ? VK_TRUE : VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = static_cast<VkBlendFactor>(_colorAttachments[i].getSrcBlendFactor());
        colorBlendAttachment.dstColorBlendFactor = static_cast<VkBlendFactor>(_colorAttachments[i].getDstBlendFactor());
        colorBlendAttachment.colorBlendOp = static_cast<VkBlendOp>(_colorAttachments[i].getColorBlendOperation());
        colorBlendAttachment.srcAlphaBlendFactor = static_cast<VkBlendFactor>(_colorAttachments[i].getSrcAlphaBlendFactor());
        colorBlendAttachment.dstAlphaBlendFactor = static_cast<VkBlendFactor>(_colorAttachments[i].getDstAlphaBlendFactor());
        colorBlendAttachment.alphaBlendOp = static_cast<VkBlendOp>(_colorAttachments[i].getAlphaBlendOperation());
        colorAttachments[i] = std::move(colorBlendAttachment);
    }

    auto colorBlendStateCI = createStruct<VkPipelineColorBlendStateCreateInfo>();
    colorBlendStateCI.logicOpEnable = static_cast<VkBool32>(_logicOperationEnable);
    colorBlendStateCI.logicOp = static_cast<VkLogicOp>(_logicOperation);
    colorBlendStateCI.attachmentCount = static_cast<decltype(colorBlendStateCI.attachmentCount)>(colorAttachments.size());
    colorBlendStateCI.pAttachments = colorAttachments.data();
    pipelineCI.pColorBlendState = &colorBlendStateCI;

    // Pipeline layout.
    using PipelineLayoutDestroyer = decltype(std::bind(vkDestroyPipelineLayout, _device, std::placeholders::_1, nullptr));
    using PipelineLayoutUniquePtr =
        std::unique_ptr<std::remove_pointer_t<VkPipelineLayout>, PipelineLayoutDestroyer>;

    auto pipelineLayoutCreateInfo = createStruct<VkPipelineLayoutCreateInfo>();
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE; // todo need to destroy.
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

