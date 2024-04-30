#include "graphicspipeline.hpp"

#include "../utils.hpp"
#include "vkutils.hpp"

#include <filesystem>

using namespace std::string_literals;

namespace avocado::vulkan {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(LogicalDevice &device):
    _logicalDevice(device),
    _pipelineLayout(device.createObjectPointer<VkPipelineLayout>(VK_NULL_HANDLE)) {
}

VkPipelineLayout GraphicsPipelineBuilder::getPipelineLayout() noexcept {
    return _pipelineLayout.get();
}

void GraphicsPipelineBuilder::addColorBlendAttachment(const VkPipelineColorBlendAttachmentState &state) {
    _colorBlendAttachments.push_back(state);
}

void GraphicsPipelineBuilder::addColorBlendAttachment(VkPipelineColorBlendAttachmentState &&state) {
    _colorBlendAttachments.emplace_back(std::move(state));
}

void GraphicsPipelineBuilder::setDynamicStates(const std::vector<VkDynamicState> &dynStates) {
    _dynamicStates = dynStates;
}

void GraphicsPipelineBuilder::setDynamicStates(std::vector<VkDynamicState> &&dynStates) {
    _dynamicStates = std::move(dynStates);
}

void GraphicsPipelineBuilder::addBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inRate) {
    _bindingDescriptions.push_back({binding, stride, inRate});
}

void GraphicsPipelineBuilder::addAttributeDescription(const uint32_t loc, const uint32_t binding, const VkFormat format, const uint32_t offset) {
    _attributeDescriptions.push_back({loc, binding, format, offset});
}

VkPipelineColorBlendStateCreateInfo& GraphicsPipelineBuilder::createColorBlendState() {
    if (_colorBlendState == nullptr) {
        _colorBlendState = std::make_unique<VkPipelineColorBlendStateCreateInfo>();
        _colorBlendState->sType = StructureType<decltype(_colorBlendState)::element_type>;
    }
    return *_colorBlendState;
}

VkPipelineDepthStencilStateCreateInfo& GraphicsPipelineBuilder::createDepthStencilState() {
    if (_depthStencilState == nullptr) {
        _depthStencilState = std::make_unique<VkPipelineDepthStencilStateCreateInfo>();
        _depthStencilState->sType = StructureType<decltype(_depthStencilState)::element_type>;
    }
    return *_depthStencilState;
}

VkPipelineDynamicStateCreateInfo& GraphicsPipelineBuilder::createDynamicState() {
    if (_dynamicState == nullptr) {
        _dynamicState = std::make_unique<VkPipelineDynamicStateCreateInfo>();
        _dynamicState->sType = StructureType<decltype(_dynamicState)::element_type>;
    }
    return *_dynamicState;
}

VkPipelineInputAssemblyStateCreateInfo& GraphicsPipelineBuilder::createInputAssmeblyState() {
    if (_inputAsmState == nullptr) {
        _inputAsmState = std::make_unique<VkPipelineInputAssemblyStateCreateInfo>();
        _inputAsmState->sType = StructureType<decltype(_inputAsmState)::element_type>;
    }
    return *_inputAsmState;
}

VkPipelineMultisampleStateCreateInfo& GraphicsPipelineBuilder::createMultisampleState() {
    if (_multisampleState == nullptr) {
        _multisampleState = std::make_unique<VkPipelineMultisampleStateCreateInfo>();
        _multisampleState->sType = StructureType<decltype(_multisampleState)::element_type>;
    }
    return *_multisampleState;
}

VkPipelineRasterizationStateCreateInfo& GraphicsPipelineBuilder::createRasterizationState() {
    if (_rasterizationState == nullptr) {
        _rasterizationState = std::make_unique<VkPipelineRasterizationStateCreateInfo>();
        _rasterizationState->sType = StructureType<decltype(_rasterizationState)::element_type>;
    }
    return *_rasterizationState;
}

VkPipelineVertexInputStateCreateInfo& GraphicsPipelineBuilder::createVertexInputState() {
    if (_vertexInputState == nullptr) {
        _vertexInputState = std::make_unique<VkPipelineVertexInputStateCreateInfo>();
        _vertexInputState->sType = StructureType<decltype(_vertexInputState)::element_type>;
    }
    return *_vertexInputState;
}

VkPipelineViewportStateCreateInfo& GraphicsPipelineBuilder::createViewportState() {
    if (_viewportState == nullptr) {
        _viewportState = std::make_unique<VkPipelineViewportStateCreateInfo>();
        _viewportState->sType = StructureType<decltype(_viewportState)::element_type>;
    }
    return *_viewportState;
}

void GraphicsPipelineBuilder::setViewPorts(const std::vector<VkViewport> &viewports) {
    _viewports = viewports;
}

void GraphicsPipelineBuilder::setViewPorts(std::vector<VkViewport> &&viewports) {
    _viewports = std::move(viewports);
}

void GraphicsPipelineBuilder::setScissors(const std::vector<VkRect2D> &scissors) {
    _scissors = scissors;
}

void GraphicsPipelineBuilder::setScissors(std::vector<VkRect2D> &&scissors) {
    _scissors = std::move(scissors);
}

void GraphicsPipelineBuilder::addFragmentShaderModules(const std::vector<std::vector<char>> &shaderModules) {
    for (std::vector<char> shaderModule: shaderModules)
        _shaderModuleCIs.emplace_back(addShaderModule(shaderModule, VK_SHADER_STAGE_FRAGMENT_BIT));
}

void GraphicsPipelineBuilder::addVertexShaderModules(const std::vector<std::vector<char>> &shaderModules) {
    for (std::vector<char> shaderModule: shaderModules)
        _shaderModuleCIs.emplace_back(addShaderModule(shaderModule, VK_SHADER_STAGE_VERTEX_BIT));
}

VkPipelineShaderStageCreateInfo GraphicsPipelineBuilder::addShaderModule(const std::vector<char> &data, const VkShaderStageFlagBits shType) {
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{}; FILL_S_TYPE(shaderStageCreateInfo);

    VkShaderModuleCreateInfo shaderModuleCreateInfo{}; FILL_S_TYPE(shaderModuleCreateInfo);
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

PipelinePtr GraphicsPipelineBuilder::createPipeline(VkRenderPass renderPass) {
    VkGraphicsPipelineCreateInfo pipelineCI{}; FILL_S_TYPE(pipelineCI);
    pipelineCI.subpass = 0;
    pipelineCI.renderPass = renderPass;

    bindStages(pipelineCI);
    createLayout(pipelineCI);
    if (hasError()) {
        setErrorMessage("Can't create pipeline layout: "s + getErrorMessage());
        return makeObjectPtr<VkPipeline>(_logicalDevice, VK_NULL_HANDLE);
    }

    // Create the pipeline.
    VkPipeline pipeline = VK_NULL_HANDLE;
    const VkResult result = vkCreateGraphicsPipelines(_logicalDevice.getHandle(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateGraphicsPipelines returned "s + getVkResultString(result));
        return makeObjectPtr<VkPipeline>(_logicalDevice, VK_NULL_HANDLE);
    }

    // Free unneeded resources.
    _shaderModuleCIs.clear();
    _shaderModuleCIs.shrink_to_fit();
    _colorBlendState.reset();
    _dynamicState.reset();
    _vertexInputState.reset();
    _inputAsmState.reset();
    _multisampleState.reset();
    _rasterizationState.reset();
    _viewportState.reset();

    return makeObjectPtr(_logicalDevice, pipeline);
}

void GraphicsPipelineBuilder::setDescriptorSetLayouts(const std::vector<VkDescriptorSetLayout> &layouts) {
    _descriptorSetLayouts = layouts;
}

void GraphicsPipelineBuilder::loadShaders(const std::string &shaderPath) {
    std::vector<std::vector<char>> fragmentShaders;
    std::vector<std::vector<char>> vertexShaders;
    for (const auto &entry: std::filesystem::directory_iterator(shaderPath)) {
        if (std::filesystem::is_regular_file(entry.path())) {
            if (avocado::utils::endsWith(entry.path().filename().string(), ".vert.spv"))
                vertexShaders.emplace_back(avocado::utils::readFile(shaderPath + "/" + entry.path().filename().string()));
            else if (avocado::utils::endsWith(entry.path().filename().string(), ".frag.spv"))
                fragmentShaders.emplace_back(avocado::utils::readFile(shaderPath + "/" + entry.path().filename().string()));
        }
    }
    addVertexShaderModules(vertexShaders);
    addFragmentShaderModules(fragmentShaders);
}

void GraphicsPipelineBuilder::bindStages(VkGraphicsPipelineCreateInfo &pipelineCreateInfo) noexcept {
    if (!_shaderModuleCIs.empty()) {
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(_shaderModuleCIs.size());
        pipelineCreateInfo.pStages = _shaderModuleCIs.data();
    }

    if (_dynamicState != nullptr) {
        if (!_dynamicStates.empty()) {
            _dynamicState->pDynamicStates = _dynamicStates.data();
            _dynamicState->dynamicStateCount = _dynamicStates.size();
        }

        pipelineCreateInfo.pDynamicState = _dynamicState.get();
    }

    if (_vertexInputState != nullptr) {
        if (!_attributeDescriptions.empty()) {
            _vertexInputState->pVertexAttributeDescriptions = _attributeDescriptions.data();
            _vertexInputState->vertexAttributeDescriptionCount = _attributeDescriptions.size();
        }

        if (!_bindingDescriptions.empty()) {
            _vertexInputState->pVertexBindingDescriptions = _bindingDescriptions.data();
            _vertexInputState->vertexBindingDescriptionCount = _bindingDescriptions.size();
        }

        pipelineCreateInfo.pVertexInputState = _vertexInputState.get();
    }

    if (_inputAsmState != nullptr)
        pipelineCreateInfo.pInputAssemblyState = _inputAsmState.get();

    if (_viewportState != nullptr) {
        if (!_scissors.empty()) {
            _viewportState->scissorCount = _scissors.size();
            _viewportState->pScissors = _scissors.data();
        }

        if (!_viewports.empty()) {
            _viewportState->viewportCount = _viewports.size();
            _viewportState->pViewports = _viewports.data();
        }

        pipelineCreateInfo.pViewportState = _viewportState.get();
    }

    if (_rasterizationState != nullptr)
        pipelineCreateInfo.pRasterizationState = _rasterizationState.get();

    if (_multisampleState != nullptr)
        pipelineCreateInfo.pMultisampleState = _multisampleState.get();

    if (_colorBlendState != nullptr) {
        if (!_colorBlendAttachments.empty()) {
            _colorBlendState->attachmentCount = _colorBlendAttachments.size();
            _colorBlendState->pAttachments = _colorBlendAttachments.data();
        }
        pipelineCreateInfo.pColorBlendState = _colorBlendState.get();
    }

    if (_depthStencilState != nullptr)
        pipelineCreateInfo.pDepthStencilState = _depthStencilState.get();
}

void GraphicsPipelineBuilder::createLayout(VkGraphicsPipelineCreateInfo &pipelineCreateInfo) noexcept {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{}; FILL_S_TYPE(pipelineLayoutCreateInfo);
    pipelineLayoutCreateInfo.setLayoutCount = _descriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = _descriptorSetLayouts.data();
    VkPipelineLayout pipelineLayout;

    const VkResult pipelineCreationResult = vkCreatePipelineLayout(_logicalDevice.getHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    setHasError(pipelineCreationResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreatePipelineLayout returned "s + getVkResultString(pipelineCreationResult));
        return;
    }

    _pipelineLayout.reset(pipelineLayout);
    pipelineCreateInfo.layout = _pipelineLayout.get();
}

} // namespace avocado::vulkan.

