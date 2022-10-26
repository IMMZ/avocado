#include "logicaldevice.hpp"

using namespace std::string_literals;

namespace avocado::vulkan {

LogicalDevice::LogicalDevice():
    _dev(VK_NULL_HANDLE, std::bind(vkDestroyDevice, std::placeholders::_1, nullptr)) {
}

LogicalDevice::LogicalDevice(VkDevice dev):
    _dev(dev, std::bind(vkDestroyDevice, std::placeholders::_1, nullptr)) {
}

VkDevice LogicalDevice::getHandle() {
    return _dev.get();
}

bool LogicalDevice::isValid() const {
    return _dev.get() != VK_NULL_HANDLE;
}

VkPipelineShaderStageCreateInfo LogicalDevice::addShaderModule(const std::vector<char> &shaderCode, ShaderType shType) {
    _shaderCodes.push_back(shaderCode);
    return createShaderModule(shType);
}

VkPipelineShaderStageCreateInfo LogicalDevice::addShaderModule(std::vector<char> &&shaderCode, ShaderType shType) {
    _shaderCodes.emplace_back(std::move(shaderCode));
    return createShaderModule(shType);
}

VkQueue LogicalDevice::getQueue(const uint32_t queueFamilyIndex, const uint32_t queueIndex) {
    VkQueue queue;
    vkGetDeviceQueue(_dev.get(), queueFamilyIndex, queueIndex, &queue);
    return queue;
}

VkFence LogicalDevice::createFence() {
    VkFenceCreateInfo fenceCI{};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    const VkResult result = vkCreateFence(_dev.get(), &fenceCI, nullptr, &fence);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateFence returned "s + getVkResultString(result));
    }
    return fence;
}

void LogicalDevice::waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout) {
    const VkResult result = vkWaitForFences(_dev.get(), static_cast<uint32_t>(fences.size()), fences.data(), waitAll ? VK_TRUE : VK_FALSE, timeout);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkWaitForFences returned "s + getVkResultString(result));
    }
}

void LogicalDevice::resetFences(const std::vector<VkFence> &fences) {
    const VkResult result = vkResetFences(_dev.get(), static_cast<uint32_t>(fences.size()), fences.data());
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkResetFences returned "s + getVkResultString(result));
    }
}

VkSemaphore LogicalDevice::createSemaphore() {
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphoreCI{};
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    const VkResult result = vkCreateSemaphore(_dev.get(), &semaphoreCI, nullptr, &semaphore);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateSemaphore returned "s + getVkResultString(result));
    }
    return semaphore;
}

VkCommandPool LogicalDevice::createCommandPool(const LogicalDevice::CommandPoolCreationFlags flags, const uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = static_cast<VkCommandPoolCreateFlags>(flags);
    poolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    const VkResult result = vkCreateCommandPool(_dev.get(), &poolCreateInfo, nullptr, &commandPool);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateCommandPool returned "s + getVkResultString(result));
    }

    return commandPool;
}


std::vector<CommandBuffer> LogicalDevice::allocateCommandBuffers(const uint32_t count, VkCommandPool cmdPool, const CommandBufferLevel bufLevel) {
    assert(count > 0);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = cmdPool;
    allocInfo.level = static_cast<VkCommandBufferLevel>(bufLevel);
    allocInfo.commandBufferCount = count;

    std::vector<VkCommandBuffer> dataToFill(count, VK_NULL_HANDLE);
    const VkResult callResult = vkAllocateCommandBuffers(_dev.get(), &allocInfo, dataToFill.data());
    setHasError(callResult != VK_SUCCESS);
    setErrorMessage("vkAllocateCommandBuffers returned "s + getVkResultString(callResult));

    std::vector<CommandBuffer> result(count);
    for (size_t i = 0; i < count; ++i)
        result[i] = CommandBuffer(dataToFill[i]);

    return result;
}

void LogicalDevice::waitIdle() {
    const VkResult result = vkDeviceWaitIdle(_dev.get());
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkDeviceWaitIdle returned "s + getVkResultString(result));
    }
}

VkPipelineShaderStageCreateInfo LogicalDevice::createShaderModule(ShaderType shType) {
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};

    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = _shaderCodes.back().size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(_shaderCodes.back().data());

    VkShaderModule shaderModule;
    const VkResult result = vkCreateShaderModule(_dev.get(), &shaderModuleCreateInfo, nullptr, &shaderModule);

    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateShaderModule returned "s + getVkResultString(result));
        return shaderStageCreateInfo;
    }

    _shaderModules.emplace_back(shaderModule,
        std::bind(vkDestroyShaderModule, _dev.get(), std::placeholders::_1, nullptr));

    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
    shaderStageCreateInfo.stage = static_cast<VkShaderStageFlagBits>(shType); 
    shaderStageCreateInfo.module = shaderModule; 
    shaderStageCreateInfo.pName = "main"; // Entry point. 
    return shaderStageCreateInfo;
}

LogicalDevice::RenderPassUniquePtr LogicalDevice::createRenderPass(VkFormat format) {
    // Attachment description.
    VkAttachmentReference attachmentRef{};
    attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentRef;

    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = format;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    VkRenderPass renderPass;
    const auto &renderPassDestroyer = std::bind(vkDestroyRenderPass, _dev.get(), std::placeholders::_1, nullptr);
    const VkResult result = vkCreateRenderPass(_dev.get(), &renderPassCreateInfo, nullptr, &renderPass);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateRenderPass returned "s + getVkResultString(result));
        return RenderPassUniquePtr(VK_NULL_HANDLE, renderPassDestroyer);
    }
    
    return RenderPassUniquePtr(renderPass, renderPassDestroyer);
}

} // namespace avocado::vulkan.

