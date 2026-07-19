#include "logicaldevice.hpp"

#include "buffer.hpp"
#include "objectdeleter.hpp"
#include "physicaldevice.hpp"
#include "vulkan/pointertypes.hpp"
#include "vkutils.hpp"

#include "../config.hpp"

#include <array>
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

void LogicalDevice::updateDescriptorSet(const std::vector<VkWriteDescriptorSet> &descriptorWriteSets) noexcept {
    vkUpdateDescriptorSets(_dev.get(), descriptorWriteSets.size(), descriptorWriteSets.data(), 0, nullptr);
}

VkDescriptorBufferInfo LogicalDevice::createDescriptorBufferInfo(Buffer &buffer, const size_t bufferSize) noexcept {
    return VkDescriptorBufferInfo {
        buffer.getHandle(),
        0,
        bufferSize};
}

DescriptorPoolPtr LogicalDevice::createDescriptorPool(const size_t descriptorCount) {
    constexpr std::array<VkDescriptorPoolSize, 1> descriptorPoolSizes {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, avocado::core::Config::MAX_BINDLESS_RESOURCES}
    };

    DEFINE_VK_STRUCTURE(VkDescriptorPoolCreateInfo, dPoolCI);
    dPoolCI.poolSizeCount = descriptorPoolSizes.size();
    dPoolCI.pPoolSizes = descriptorPoolSizes.data();
    dPoolCI.maxSets = static_cast<uint32_t>(descriptorCount);
    dPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    CALL_VULKAN_AND_RETURN_VALUE(
        createObjectPointer<VkDescriptorPool>(VK_NULL_HANDLE),
        vkCreateDescriptorPool,_dev.get(), &dPoolCI, nullptr, &descriptorPool);

    return createObjectPointer(descriptorPool);
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

void LogicalDevice::setQueueFamilies(const QueueFamily graphicsQueueFamily, const QueueFamily presentQueueFamily, const QueueFamily transferQueueFamily) noexcept {
    _graphicsQueueFamily = graphicsQueueFamily;
    _presentQueueFamily = presentQueueFamily;
    _transferQueueFamily = transferQueueFamily;
}

FencePtr LogicalDevice::createFence() noexcept {
    DEFINE_VK_STRUCTURE(VkFenceCreateInfo, fenceCI);
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    CALL_VULKAN(vkCreateFence, _dev.get(), &fenceCI, nullptr, &fence);

    return createObjectPointer(fence);
}

void LogicalDevice::waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout) noexcept {
    CALL_VULKAN(vkWaitForFences, _dev.get(), static_cast<uint32_t>(fences.size()), fences.data(), waitAll ? VK_TRUE : VK_FALSE, timeout);
}

void LogicalDevice::resetFences(const std::vector<VkFence> &fences) noexcept {
    CALL_VULKAN(vkResetFences, _dev.get(), static_cast<uint32_t>(fences.size()), fences.data());
}

SemaphorePtr LogicalDevice::createSemaphore() noexcept {
    VkSemaphore semaphore;
    DEFINE_VK_STRUCTURE(VkSemaphoreCreateInfo, semaphoreCI);
    CALL_VULKAN(vkCreateSemaphore, _dev.get(), &semaphoreCI, nullptr, &semaphore);

    return createObjectPointer(semaphore);
}

void LogicalDevice::waitIdle() noexcept {
    CALL_VULKAN(vkDeviceWaitIdle, _dev.get());
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
    DEFINE_VK_STRUCTURE(VkRenderPassCreateInfo, renderPassCreateInfo);
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    VkRenderPass renderPass;
    CALL_VULKAN_AND_RETURN_VALUE(
        RenderPassPtr(makeObjectPtr<VkRenderPass>(*this, VK_NULL_HANDLE)),
        vkCreateRenderPass,_dev.get(), &renderPassCreateInfo, nullptr, &renderPass);

    return RenderPassPtr(makeObjectPtr(*this, renderPass));
}

} // namespace avocado::vulkan.

