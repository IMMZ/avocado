#include "swapchain.hpp"

#include "logicaldevice.hpp"
#include "surface.hpp"
#include "vkutils.hpp"

#include <cassert>

using namespace std::string_literals;

namespace avocado::vulkan {

Swapchain::Swapchain(LogicalDevice &logicalDevice):
    _device(logicalDevice.getHandle()){
}

Swapchain::~Swapchain() {
    if (isValid()) {
        for (VkFramebuffer fb: _framebuffers)
            vkDestroyFramebuffer(_device, fb, nullptr);
        for (VkImageView iv: _imageViews)
            vkDestroyImageView(_device, iv, nullptr);
        // todo Do we need to remove images?
        //for (VkImage img: _images)
         //   vkDestroyImage(_device, img, nullptr);
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
    }
}

// todo Should we destroy objects in vectors?
Swapchain::Swapchain(Swapchain &&other):
    _images(std::move(other._images)),
    _imageViews(std::move(other._imageViews)),
    _framebuffers(std::move(other._framebuffers)),
    _device(std::move(other._device)),
    _swapchain(std::move(other._swapchain)){
    other._swapchain = VK_NULL_HANDLE;
}

Swapchain& Swapchain::operator=(Swapchain &&other) {
    _images = std::move(other._images);
    _imageViews = std::move(other._imageViews);
    _framebuffers = std::move(other._framebuffers);
    _device = std::move(other._device);
    _swapchain = std::move(other._swapchain);
    other._swapchain = VK_NULL_HANDLE;
    return *this;
}

VkSwapchainKHR Swapchain::getHandle() noexcept {
    return _swapchain;
}

bool Swapchain::isValid() const noexcept {
    return (_swapchain != VK_NULL_HANDLE);
}

void Swapchain::create(Surface &surface, VkSurfaceFormatKHR surfaceFormat, VkExtent2D extent, const uint32_t minImageCount, const std::vector<QueueFamily> &queueFamilies) noexcept {
    auto swapchainCreateInfo = createStruct<VkSwapchainCreateInfoKHR>();
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

    if (queueFamilies.size() > 1) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    const VkResult result = vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapchain);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateSwapchainKHR returned "s + getVkResultString(result));
    }
}

void Swapchain::getImages() {
    assert(_swapchain != VK_NULL_HANDLE);

    uint32_t imageCount = std::numeric_limits<uint32_t>::max();
    const VkResult result = vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkGetSwapchainImagesKHR returned "s + getVkResultString(result));
        return;
    }

    _images.resize(imageCount);
    const VkResult result2 = vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _images.data());
    setHasError(result2 != VK_SUCCESS);

    if (hasError()) {
        setErrorMessage("vkGetSwapchainImagesKHR returned "s + getVkResultString(result2));
        return;
    }
}

void Swapchain::createImageViews(VkSurfaceFormatKHR surfaceFormat) {
    assert(_swapchain != VK_NULL_HANDLE);

    _imageViews.resize(_images.size());
    for (size_t i = 0; i < _images.size(); ++i) {
        auto createInfo = createStruct<VkImageViewCreateInfo>();
        createInfo.image = _images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = surfaceFormat.format;
        createInfo.components.r = createInfo.components.g
            = createInfo.components.b = createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        const VkResult result = vkCreateImageView(_device, &createInfo, nullptr, &_imageViews[i]);
        setHasError(result != VK_SUCCESS);
        if (hasError()) {
            setErrorMessage("vkCreateImageView returned "s + getVkResultString(result));
            return;
        }
    }
}

void Swapchain::createFramebuffers(VkRenderPass renderPass, VkExtent2D extent) {
    assert(_swapchain != VK_NULL_HANDLE);

    _framebuffers.resize(_images.size());
    for (size_t i = 0; i < _imageViews.size(); i++) {
        VkImageView attachments[] = {_imageViews[i]};
        auto framebufferInfo = createStruct<VkFramebufferCreateInfo>();
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;
        const VkResult result = vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_framebuffers[i]);
        setHasError(result != VK_SUCCESS);
        if (hasError()) {
            setErrorMessage("vkCreateFramebuffer returned "s +  getVkResultString(result));
        }
    }
}

uint32_t Swapchain::acquireNextImage(VkSemaphore semaphore) noexcept {
    assert(_swapchain != VK_NULL_HANDLE);

    auto imageIndex = std::numeric_limits<uint32_t>::max();
    const VkResult result = vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex);
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

