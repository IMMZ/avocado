#include "logicaldevice.hpp"

#include "buffer.hpp"
#include "debugutils.hpp"
#include "objectdeleter.hpp"
#include "physicaldevice.hpp"
#include "vkutils.hpp"
#include "vulkan_core.h"

#include <cstdint>
#include <memory>

using namespace std::string_literals;

namespace avocado::vulkan {

LogicalDevice::LogicalDevice():
    _dev(makeFundamentalObjectPtr<VkDevice>(VK_NULL_HANDLE)) {
}

LogicalDevice::LogicalDevice(VkDevice dev):
    _dev(makeFundamentalObjectPtr(dev)) {
}

VkDevice LogicalDevice::getHandle() noexcept {
    return _dev.get();
}

bool LogicalDevice::isValid() const noexcept {
    return _dev.get() != VK_NULL_HANDLE;
}

VkDescriptorSetLayoutBinding LogicalDevice::createLayoutBinding(const uint32_t bindingNumber, const VkDescriptorType type,
    const uint32_t descriptorCount, const VkShaderStageFlags flags, const std::vector<VkSampler> &samplers) noexcept {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = bindingNumber;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = descriptorCount;
    layoutBinding.stageFlags = flags;
    if (!samplers.empty())
        layoutBinding.pImmutableSamplers = samplers.data();
    return layoutBinding;
}

VkDescriptorSetLayout LogicalDevice::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
    assert(_dev != nullptr && "Device handle mustn't be null.");

    VkDescriptorSetLayoutCreateInfo createInfo{}; FILL_S_TYPE(createInfo);
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();

    VkDescriptorSetLayout descriptorSetLayout;
    const VkResult result = vkCreateDescriptorSetLayout(_dev.get(), &createInfo, nullptr, &descriptorSetLayout);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateDescriptorSetLayout returned "s + getVkResultString(result));
        return VK_NULL_HANDLE;
    }

    return descriptorSetLayout;
}

void LogicalDevice::updateDescriptorSet(const std::vector<VkWriteDescriptorSet> &descriptorWriteSets) noexcept {
    vkUpdateDescriptorSets(_dev.get(), descriptorWriteSets.size(), descriptorWriteSets.data(), 0, nullptr);
}

VkDescriptorBufferInfo LogicalDevice::createDescriptorBufferInfo(Buffer &buffer, const size_t bufferSize) noexcept {
    return VkDescriptorBufferInfo {
        buffer.getHandle(),
        0,
        bufferSize};
}

std::pair<VkDescriptorSet, VkWriteDescriptorSet> LogicalDevice::createWriteDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorBufferInfo &descriptorBufferInfo) {
    VkWriteDescriptorSet descriptorWrite{}; FILL_S_TYPE(descriptorWrite);
    VkDescriptorSetAllocateInfo allocInfo{}; FILL_S_TYPE(allocInfo);
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet dSet;
    const VkResult ads = vkAllocateDescriptorSets(_dev.get(), &allocInfo, &dSet);
    setHasError(ads != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkAllocateDescriptorSets returned "s + getVkResultString(ads));
        return {dSet, descriptorWrite};
    }

    descriptorWrite.dstSet = dSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &descriptorBufferInfo;
    return {dSet, descriptorWrite};
}

VkDescriptorPool LogicalDevice::createDescriptorPool(const size_t descriptorCount) {
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes{};
    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[0].descriptorCount = descriptorCount;
    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[1].descriptorCount = descriptorCount;

    VkDescriptorPoolCreateInfo dPoolCI{}; FILL_S_TYPE(dPoolCI);
    dPoolCI.poolSizeCount = descriptorPoolSizes.size();
    dPoolCI.pPoolSizes = descriptorPoolSizes.data();
    dPoolCI.maxSets = static_cast<uint32_t>(descriptorCount);

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    const VkResult cdp = vkCreateDescriptorPool(_dev.get(), &dPoolCI, nullptr, &descriptorPool);
    setHasError(cdp != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateDescriptorPool returned "s + getVkResultString(cdp));
        return VK_NULL_HANDLE;
    }
    return descriptorPool;
}

Queue LogicalDevice::getGraphicsQueue(const uint32_t index) noexcept {
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(_dev.get(), _graphicsQueueFamily, index, &queue);
    return Queue(queue);
}

Queue LogicalDevice::getPresentQueue(const uint32_t index) noexcept {
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(_dev.get(), _presentQueueFamily, index, &queue);
    return Queue(queue);
}

Queue LogicalDevice::getTransferQueue(const uint32_t index) noexcept {
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(_dev.get(), _presentQueueFamily, index, &queue);
    return Queue(queue);
}

std::unique_ptr<DebugUtils> LogicalDevice::createDebugUtils() {
    auto *debugUtils = new DebugUtils(*this);
    return std::unique_ptr<DebugUtils>(debugUtils);
}

void LogicalDevice::setQueueFamilies(const QueueFamily graphicsQueueFamily, const QueueFamily presentQueueFamily, const QueueFamily transferQueueFamily) noexcept {
    _graphicsQueueFamily = graphicsQueueFamily;
    _presentQueueFamily = presentQueueFamily;
    _transferQueueFamily = transferQueueFamily;
}

VkFence LogicalDevice::createFence() noexcept {
    VkFenceCreateInfo fenceCI{}; FILL_S_TYPE(fenceCI);
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    const VkResult result = vkCreateFence(_dev.get(), &fenceCI, nullptr, &fence);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateFence returned "s + getVkResultString(result));
    }
    return fence;
}

void LogicalDevice::waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout) noexcept {
    const VkResult result = vkWaitForFences(_dev.get(), static_cast<uint32_t>(fences.size()), fences.data(), waitAll ? VK_TRUE : VK_FALSE, timeout);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkWaitForFences returned "s + getVkResultString(result));
    }
}

void LogicalDevice::resetFences(const std::vector<VkFence> &fences) noexcept {
    const VkResult result = vkResetFences(_dev.get(), static_cast<uint32_t>(fences.size()), fences.data());
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkResetFences returned "s + getVkResultString(result));
    }
}

VkSemaphore LogicalDevice::createSemaphore() noexcept {
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphoreCI{}; FILL_S_TYPE(semaphoreCI);
    const VkResult result = vkCreateSemaphore(_dev.get(), &semaphoreCI, nullptr, &semaphore);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkCreateSemaphore returned "s + getVkResultString(result));

    return semaphore;
}

SamplerPtr LogicalDevice::createSampler(PhysicalDevice &physicalDevice) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.anisotropyEnable = VK_FALSE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice.getHandle(), &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler textureSampler;
    const VkResult samplerCreateResult = vkCreateSampler(_dev.get(), &samplerInfo, nullptr, &textureSampler);
    setHasError(samplerCreateResult != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkCreateSampler returned "s + getVkResultString(samplerCreateResult));

    return createObjectPointer(textureSampler);
}

VkCommandPool LogicalDevice::createCommandPool(const VkCommandPoolCreateFlags flags, const QueueFamily queueFamilyIndex) noexcept {
    VkCommandPoolCreateInfo poolCreateInfo{}; FILL_S_TYPE(poolCreateInfo);
    poolCreateInfo.flags = flags;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    const VkResult result = vkCreateCommandPool(_dev.get(), &poolCreateInfo, nullptr, &commandPool);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateCommandPool returned "s + getVkResultString(result));
    }

    return commandPool;
}

std::vector<CommandBuffer> LogicalDevice::allocateCommandBuffers(const uint32_t count, VkCommandPool cmdPool, const VkCommandBufferLevel bufLevel) {
    assert(count > 0 && "Command buffer count must be > 0.");

    VkCommandBufferAllocateInfo allocInfo{}; FILL_S_TYPE(allocInfo);
    allocInfo.commandPool = cmdPool;
    allocInfo.level = static_cast<VkCommandBufferLevel>(bufLevel);
    allocInfo.commandBufferCount = count;

    std::vector<VkCommandBuffer> dataToFill(count, VK_NULL_HANDLE);
    const VkResult callResult = vkAllocateCommandBuffers(_dev.get(), &allocInfo, dataToFill.data());
    setHasError(callResult != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkAllocateCommandBuffers returned "s + getVkResultString(callResult));

    std::vector<CommandBuffer> result(count);
    for (size_t i = 0; i < count; ++i)
        result[i] = CommandBuffer(dataToFill[i]);

    return result;
}

void LogicalDevice::waitIdle() noexcept {
    const VkResult result = vkDeviceWaitIdle(_dev.get());
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkDeviceWaitIdle returned "s + getVkResultString(result));
    }
}

RenderPassPtr LogicalDevice::createRenderPass(VkFormat format, VkFormat depthFormat) {
    // Attachment description.
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassCreateInfo{}; FILL_S_TYPE(renderPassCreateInfo);
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    VkRenderPass renderPass;
    const VkResult result = vkCreateRenderPass(_dev.get(), &renderPassCreateInfo, nullptr, &renderPass);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateRenderPass returned "s + getVkResultString(result));
        return RenderPassPtr(makeObjectPtr<VkRenderPass>(*this, VK_NULL_HANDLE));
    }

    return RenderPassPtr(makeObjectPtr(*this, renderPass));
}

} // namespace avocado::vulkan.

