#include "graphicspipeline.hpp"

#include "vkutils.hpp"


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

void GraphicsPipelineBuilder::setColorBlendState(std::unique_ptr<ColorBlendState> state) noexcept {
    _colorBlendState = std::move(state);
}

void GraphicsPipelineBuilder::setDynamicState(std::unique_ptr<DynamicState> dynState) noexcept {
    _dynamicState = std::move(dynState);
}

void GraphicsPipelineBuilder::setInputAsmState(std::unique_ptr<InputAsmState> inAsmState) noexcept {
    _inputAsmState = std::move(inAsmState);
}

void GraphicsPipelineBuilder::setMultisampleState(std::unique_ptr<MultisampleState> mss) noexcept {
    _multisampleState = std::move(mss);
}

void GraphicsPipelineBuilder::setRasterizationState(std::unique_ptr<RasterizationState> rastState) noexcept {
    _rasterizationState = std::move(rastState);
}

void GraphicsPipelineBuilder::setVertexInputState(std::unique_ptr<VertexInputState> vertexInState) noexcept {
    _vertexInputState = std::move(vertexInState);
}

void GraphicsPipelineBuilder::setViewportState(std::unique_ptr<ViewportState> viewportState) noexcept {
    _viewportState = std::move(viewportState);
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

    _shaderModules.emplace_back(makeObjectPtr(_logicalDevice, shaderModule));

    shaderStageCreateInfo.stage = shType;
    shaderStageCreateInfo.module = _shaderModules.back().get();
    shaderStageCreateInfo.pName = "main"; // Entry point.
    return shaderStageCreateInfo;
}

PipelinePtr GraphicsPipelineBuilder::buildPipeline(VkRenderPass renderPass) {
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
        return makeObjectPtr<VkPipeline>(_logicalDevice, VK_NULL_HANDLE);
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
    _colorBlendState = nullptr;
    _dynamicState = nullptr;
    _vertexInputState = nullptr;
    _inputAsmState = nullptr;
    _multisampleState = nullptr;
    _rasterizationState = nullptr;
    _viewportState = nullptr;

    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateGraphicsPipelines returned "s + getVkResultString(result));
        return makeObjectPtr<VkPipeline>(_logicalDevice, VK_NULL_HANDLE);
    }

    return makeObjectPtr(_logicalDevice, pipeline);
}

void GraphicsPipelineBuilder::setDSLayouts(std::vector<VkDescriptorSetLayout> &layouts)
{
    _dsLayouts = layouts;
}

void GraphicsPipelineBuilder::destroyPipeline() {
    vkDestroyDescriptorSetLayout(_logicalDevice.getHandle(), _descriptorSetLayout, nullptr);
}

} // namespace avocado::vulkan.

