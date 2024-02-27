#include "logicaldevice.hpp"

#include "debugutils.hpp"
#include "vkutils.hpp"

#include <memory>

using namespace std::string_literals;

namespace avocado::vulkan {

LogicalDevice::LogicalDevice():
    _dev(VK_NULL_HANDLE, std::bind(vkDestroyDevice, std::placeholders::_1, nullptr)) {
}

LogicalDevice::LogicalDevice(VkDevice dev):
    _dev(dev, std::bind(vkDestroyDevice, std::placeholders::_1, nullptr)) {
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
    assert(_dev != nullptr);

    auto createInfo = createStruct<VkDescriptorSetLayoutCreateInfo>();
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
    auto fenceCI = createStruct<VkFenceCreateInfo>();
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
    auto semaphoreCI = createStruct<VkSemaphoreCreateInfo>();
    const VkResult result = vkCreateSemaphore(_dev.get(), &semaphoreCI, nullptr, &semaphore);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateSemaphore returned "s + getVkResultString(result));
    }
    return semaphore;
}

VkCommandPool LogicalDevice::createCommandPool(const LogicalDevice::CommandPoolCreationFlags flags, const QueueFamily queueFamilyIndex) noexcept {
    auto poolCreateInfo = createStruct<VkCommandPoolCreateInfo>();
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

    auto allocInfo = createStruct<VkCommandBufferAllocateInfo>();
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

// todo Refactor? Extract constants as parameters.
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

    auto renderPassCreateInfo = createStruct<VkRenderPassCreateInfo>();
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

