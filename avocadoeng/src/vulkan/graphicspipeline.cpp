#include "graphicspipeline.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cassert>

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

// todo Should it be in this class?
VkViewport GraphicsPipelineBuilder::getViewport() const {
    static VkViewport viewPort{}; // todo remove static
    viewPort.width = static_cast<float>(_viewPortWidth);
    viewPort.height = static_cast<float>(_viewPortHeight);
    viewPort.maxDepth = _viewPortMaxDepth;
    return viewPort;
}

VkRect2D GraphicsPipelineBuilder::getScissor() const {
    static VkRect2D scissor{};
    scissor.extent = {_viewPortWidth, _viewPortHeight};
    return scissor;
}

void GraphicsPipelineBuilder::setViewPortSize(const uint32_t w, const uint32_t h) {
    _viewPortWidth = w; _viewPortHeight = h;
}

void GraphicsPipelineBuilder::setViewPortMaxDepth(const float maxDepth) {
    _viewPortMaxDepth = maxDepth;
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


GraphicsPipelineBuilder::PipelineUniquePtr GraphicsPipelineBuilder::buildPipeline(const std::vector<VkPipelineShaderStageCreateInfo> &shaderStageCIs, VkFormat format, VkRenderPass renderPass) {
    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // Shader modules.
    pipelineCI.stageCount = static_cast<uint32_t>(shaderStageCIs.size());
    pipelineCI.pStages = shaderStageCIs.data();

    // Dynamic states.
    VkPipelineDynamicStateCreateInfo dynamicStateCI{};
    dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size());
    dynamicStateCI.pDynamicStates = _dynamicStates.data();
    pipelineCI.pDynamicState = &dynamicStateCI;

    // Vertex input.
    VkPipelineVertexInputStateCreateInfo vertexInStateCreateInfo{};
    vertexInStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineCI.pVertexInputState = &vertexInStateCreateInfo;

    // Input assembly.
    VkPipelineInputAssemblyStateCreateInfo inputAsmStateCI{};
    inputAsmStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
    inputAsmStateCI.topology = static_cast<VkPrimitiveTopology>(_primitiveTopology); 
    pipelineCI.pInputAssemblyState = &inputAsmStateCI;

    // Viewport.
    //VkViewport viewPort = getViewport();
    //VkRect2D scissor = getScissor();
    auto viewPort = new VkViewport(getViewport());
    auto scissor = new VkRect2D(getScissor());

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1; 
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pViewports = viewPort;
    viewportStateCreateInfo.pScissors = scissor;
    pipelineCI.pViewportState = &viewportStateCreateInfo;
    
    // Rasterizer.
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = static_cast<VkBool32>(_depthClamEnable);
    rasterizerCreateInfo.rasterizerDiscardEnable = static_cast<VkBool32>(_rasterizerDiscardEnable);
    rasterizerCreateInfo.polygonMode = static_cast<VkPolygonMode>(_polygonMode);
    rasterizerCreateInfo.lineWidth = _lineWidth;
    rasterizerCreateInfo.cullMode = static_cast<VkFlags>(_cullMode);
    rasterizerCreateInfo.frontFace = static_cast<VkFrontFace>(_frontFace);
    rasterizerCreateInfo.depthBiasEnable = static_cast<VkBool32>(_depthBiasEnable);
    pipelineCI.pRasterizationState = &rasterizerCreateInfo;

    // Multisampling.
    VkPipelineMultisampleStateCreateInfo multisamplingCI{};
    multisamplingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
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

    VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
    colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCI.logicOpEnable = static_cast<VkBool32>(_logicOperationEnable);
    colorBlendStateCI.logicOp = static_cast<VkLogicOp>(_logicOperation);
    colorBlendStateCI.attachmentCount = static_cast<decltype(colorBlendStateCI.attachmentCount)>(colorAttachments.size());
    colorBlendStateCI.pAttachments = colorAttachments.data();
    pipelineCI.pColorBlendState = &colorBlendStateCI;

    // Pipeline layout.

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkPipelineLayout pipelineLayout; // todo need to destroy.
    const VkResult pipelineCreationResult = vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    auto pipelineDestroyer = std::bind(vkDestroyPipeline, _device, std::placeholders::_1, nullptr);
    if (pipelineCreationResult != VK_SUCCESS) {
        setHasError(true);
        // todo uncomment and fix
        // setErrorMessage("vkCreatePipelineLayout() returned "s + internal::getVkResultString(pipelineCreationResult));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }

    // todo PipelineLayoutUniquePtr pipelineLayoutPtr(pipelineLayout, pipelineLayoutDeleter);
    pipelineCI.layout = pipelineLayout; //pipelineLayoutPtr.get();

    pipelineCI.renderPass = renderPass;

    // Create the pipeline.
    VkPipeline pipeline = VK_NULL_HANDLE;
    const VkResult result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);
    if (result != VK_SUCCESS) {
        setHasError(true);
        // todo uncomment and fix
        // setErrorMessage("vkCreateGraphicsPipelines returned "s + internal::getVkResultString(result));
        return PipelineUniquePtr(VK_NULL_HANDLE, pipelineDestroyer);
    }

    return PipelineUniquePtr(pipeline, pipelineDestroyer);
}

} // namespace avocado::vulkan.

