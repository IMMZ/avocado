#include "swapchain.hpp"

#include "logicaldevice.hpp"
#include "surface.hpp"
#include "vkutils.hpp"

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
    // todo check for self-assignment
    _images = std::move(other._images);
    _imageViews = std::move(other._imageViews);
    _framebuffers = std::move(other._framebuffers);
    _device = std::move(other._device);
    _swapchain = std::move(other._swapchain);
    other._swapchain = VK_NULL_HANDLE;
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
    assert(_swapchain != VK_NULL_HANDLE);

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

VkImageView Swapchain::createImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo createInfo{}; FILL_S_TYPE(createInfo);
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = createInfo.components.g
        = createInfo.components.b = createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
    assert(_swapchain != VK_NULL_HANDLE);

    _imageViews.resize(_images.size());
    for (size_t i = 0; i < _images.size(); ++i) {
        _imageViews[i] = createImageView(_images[i], surfaceFormat.format);
        if (hasError()) {
            return;
        }
    }
}

void Swapchain::createFramebuffers(VkRenderPass renderPass, VkExtent2D extent) {
    assert(_swapchain != VK_NULL_HANDLE);

    _framebuffers.resize(_images.size());
    for (size_t i = 0; i < _imageViews.size(); i++) {
        VkImageView attachments[] = {_imageViews[i]}; // todo Do we really need this line? We could take a pointer to _imageViews[i] directly.
        VkFramebufferCreateInfo framebufferCI{}; FILL_S_TYPE(framebufferCI);
        framebufferCI.renderPass = renderPass;
        framebufferCI.attachmentCount = 1;
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
    assert(_swapchain != VK_NULL_HANDLE);

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

