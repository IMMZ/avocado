#include "swapchain.hpp"

#include "logicaldevice.hpp"
#include "physicaldevice.hpp"
#include "surface.hpp"
#include "vkutils.hpp"
#include "vulkan_core.h"

#include <cassert>

using namespace std::string_literals;

namespace avocado::vulkan {

Swapchain::Swapchain(LogicalDevice &logicalDevice):
    _device(logicalDevice.getHandle()),
    _swapchain(logicalDevice.createObjectPointer<VkSwapchainKHR>(VK_NULL_HANDLE)) {
}

Swapchain::~Swapchain() {
    if (isValid()) {
        for (VkFramebuffer fb: _framebuffers)
            vkDestroyFramebuffer(_device, fb, nullptr);
        for (VkImageView iv: _imageViews)
            vkDestroyImageView(_device, iv, nullptr);

        vkFreeMemory(_device, _depthImageMemory, nullptr);
        vkDestroyImageView(_device, _depthImageView, nullptr);
        vkDestroyImage(_device, _depthImage, nullptr);
    }
}

// todo Should we destroy objects in vectors?
Swapchain::Swapchain(Swapchain &&other):
    _images(std::move(other._images)),
    _imageViews(std::move(other._imageViews)),
    _framebuffers(std::move(other._framebuffers)),
    _device(std::move(other._device)),
    _swapchain(std::move(other._swapchain)) {
    other._swapchain = VK_NULL_HANDLE;
}

Swapchain& Swapchain::operator=(Swapchain &&other) {
    if (this != &other) {
        _images = std::move(other._images);
        _imageViews = std::move(other._imageViews);
        _framebuffers = std::move(other._framebuffers);
        _device = std::move(other._device);
        _swapchain = std::move(other._swapchain);
        other._swapchain = VK_NULL_HANDLE;
    }
    return *this;
}

VkSwapchainKHR Swapchain::getHandle() noexcept {
    return _swapchain.get();
}

bool Swapchain::isValid() const noexcept {
    return (_swapchain != VK_NULL_HANDLE);
}

void Swapchain::create(Surface &surface, VkSurfaceFormatKHR surfaceFormat, VkExtent2D extent, const uint32_t minImageCount, const std::vector<QueueFamily> &queueFamilies) noexcept {
    VkSwapchainCreateInfoKHR swapchainCreateInfo{}; FILL_S_TYPE(swapchainCreateInfo);
    swapchainCreateInfo.surface = surface.getHandle();
    swapchainCreateInfo.minImageCount = minImageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = surface.getCurrentTransform();
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilies.data();
    swapchainCreateInfo.imageSharingMode = (queueFamilies.size() > 1) ?
        VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    const VkResult result = vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &swapchain);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateSwapchainKHR returned "s + getVkResultString(result));
        return;
    }
    _swapchain.reset(swapchain);
}

void Swapchain::getImages() {
    assert(_swapchain != VK_NULL_HANDLE && "Handle mustn't be null.");

    uint32_t imageCount = std::numeric_limits<uint32_t>::max();
    const VkResult result = vkGetSwapchainImagesKHR(_device, _swapchain.get(), &imageCount, nullptr);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkGetSwapchainImagesKHR returned "s + getVkResultString(result));
        return;
    }

    _images.resize(imageCount);
    const VkResult result2 = vkGetSwapchainImagesKHR(_device, _swapchain.get(), &imageCount, _images.data());
    setHasError(result2 != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkGetSwapchainImagesKHR returned "s + getVkResultString(result2));
        return;
    }
}

VkFormat Swapchain::findSupportedFormat(const std::vector<VkFormat>& candidates, VkPhysicalDevice physDevice, const VkImageTiling tiling, const VkFormatFeatureFlags features) {
    for (VkFormat format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    return VK_FORMAT_MAX_ENUM;
}

void Swapchain::createDepthImage(const uint32_t imgW, const uint32_t imgH, PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags) {
    const VkFormat format = findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        physDevice.getHandle(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkImageCreateInfo imageCreateInfo{}; FILL_S_TYPE(imageCreateInfo);
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.extent.width = imgW;
    imageCreateInfo.extent.height = imgH;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult result = vkCreateImage(_device, &imageCreateInfo, nullptr, &_depthImage);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateImage returned "s );
        return;
    }

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(_device, _depthImage, &memRequirements);

    const uint32_t foundType = physDevice.findMemoryTypeIndex(memoryFlags, memRequirements.memoryTypeBits);

    VkMemoryAllocateInfo imageMemAI{}; FILL_S_TYPE(imageMemAI);
    imageMemAI.allocationSize = memRequirements.size;
    imageMemAI.memoryTypeIndex = foundType;

    const VkResult allocationResult = vkAllocateMemory(_device, &imageMemAI, nullptr, &_depthImageMemory);
    setHasError(allocationResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkAllocateMemory returned "s + getVkResultString(allocationResult));
        return;
    }

    const VkResult bindResult = vkBindImageMemory(_device, _depthImage, _depthImageMemory, 0);
    setHasError(bindResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkBindImageMemory returned "s + getVkResultString(bindResult));
        return;
    }

    _depthImageView = createImageView(_depthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkImage Swapchain::getDepthImage() noexcept {
    return _depthImage;
}

VkImageView Swapchain::createImageView(VkImage image, VkFormat format, const VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo createInfo{}; FILL_S_TYPE(createInfo);
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = createInfo.components.g
        = createInfo.components.b = createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView = VK_NULL_HANDLE;
    const VkResult result = vkCreateImageView(_device, &createInfo, nullptr, &imageView);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateImageView returned "s + getVkResultString(result));
    }

    return imageView;
}

void Swapchain::createImageViews(VkSurfaceFormatKHR surfaceFormat) {
    assert(_swapchain != VK_NULL_HANDLE && "Handle mustn't be null.");

    _imageViews.resize(_images.size());
    for (size_t i = 0; i < _images.size(); ++i) {
        _imageViews[i] = createImageView(_images[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
        if (hasError()) {
            return;
        }
    }
}

void Swapchain::createFramebuffers(VkRenderPass renderPass, VkExtent2D extent) {
    assert(_swapchain != VK_NULL_HANDLE && "Handle mustn't be null.");

    _framebuffers.resize(_images.size());
    for (size_t i = 0; i < _imageViews.size(); i++) {
        VkImageView attachments[] = {_imageViews[i], _depthImageView};
        VkFramebufferCreateInfo framebufferCI{}; FILL_S_TYPE(framebufferCI);
        framebufferCI.renderPass = renderPass;
        framebufferCI.attachmentCount = 2;
        framebufferCI.pAttachments = attachments;
        framebufferCI.width = extent.width;
        framebufferCI.height = extent.height;
        framebufferCI.layers = 1;
        const VkResult result = vkCreateFramebuffer(_device, &framebufferCI, nullptr, &_framebuffers[i]);
        setHasError(result != VK_SUCCESS);
        if (hasError()) {
            setErrorMessage("vkCreateFramebuffer returned "s +  getVkResultString(result));
        }
    }
}

uint32_t Swapchain::acquireNextImage(VkSemaphore semaphore) noexcept {
    assert(_swapchain != VK_NULL_HANDLE && "Handle mustn't be null.");

    auto imageIndex = std::numeric_limits<uint32_t>::max();
    const VkResult result = vkAcquireNextImageKHR(_device, _swapchain.get(), UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkAcquireNextImageKHR returned "s + getVkResultString(result));
    }
    return imageIndex;
}

VkFramebuffer Swapchain::getFramebuffer(size_t index) {
    return _framebuffers[index];
}

}

