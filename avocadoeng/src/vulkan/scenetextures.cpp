#include "scenetextures.hpp"

#include "descriptormanager.hpp"
#include "physicaldevice.hpp"
#include "queuemanager.hpp"
#include "structuretypes.hpp"
#include "swapchain.hpp"

#include "../config.hpp"
#include "../scene/sampler.hpp"
#include "../scene/scenemanager.hpp"

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

SceneTextures::SceneTextures(PhysicalDevice &physicalDevice, LogicalDevice &logicalDevice):
    _physicalDevice(physicalDevice),
    _logicalDevice(logicalDevice) {

}

void SceneTextures::load(const core::SceneManager &sceneManager, Swapchain &swapchain, QueueManager &queueManager) {
    for (size_t i = 0; i < sceneManager._textures.size(); ++i) {
        loadSampler(sceneManager._textures[i]);
        loadImage(sceneManager._textures[i], swapchain);

        // Set up mappings.
        if (sceneManager._textures[i]._sampler != nullptr)
            _samplersMapping.push_back(_samplers.size() - 1);
        else
            _samplersMapping.push_back(std::numeric_limits<size_t>::max());


        if (sceneManager._textures[i]._image != nullptr)
            _imagesMapping.push_back(_images.size() - 1);
        else
            _imagesMapping.push_back(std::numeric_limits<size_t>::max());
    }

    queueManager.imageTransitionBarriers(_images, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
    queueManager.copyBuffersToImages(_buffers, _images);
    queueManager.imageTransitionBarriers(_images, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

    _buffers.clear();
}

void SceneTextures::exportToDescriptorManager(DescriptorManager &descriptorManager) {
    for (size_t i = 0; i < _imagesMapping.size(); ++i) {
        const bool hasImage = (_imagesMapping[i] != std::numeric_limits<size_t>::max());
        const bool hasSampler = (_samplersMapping[i] != std::numeric_limits<size_t>::max());
        const bool hasSamplerAndImage = hasImage && hasSampler;
        if (hasSamplerAndImage) {
            descriptorManager.addImage(_imageViews[_imagesMapping[i]], _samplers[_samplersMapping[i]], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            descriptorManager.addImage(_imageViews[_imagesMapping[i]], _samplers[_samplersMapping[i]], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
}

void SceneTextures::loadImage(const core::Texture &texture, Swapchain &swapchain) {
    if (texture._image != nullptr) {
        const core::Image &image = *texture._image;
        vulkan::Image textureImage(_logicalDevice, image._width, image._height, VK_IMAGE_TYPE_2D);
        textureImage.setDepth(1);
        textureImage.setFormat(VK_FORMAT_R8G8B8A8_SRGB);
        textureImage.setMipLevels(1);
        textureImage.setImageTiling(VK_IMAGE_TILING_OPTIMAL);
        textureImage.setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        textureImage.setSampleCount(VK_SAMPLE_COUNT_1_BIT);
        textureImage.setArrayLayerCount(1);
        textureImage.setSharingMode(VK_SHARING_MODE_EXCLUSIVE);
        textureImage.create();
        // todo process error
        //if (textureImage.hasError()) {
        textureImage.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        textureImage.bindMemory();

        _imageViews.push_back(_logicalDevice.createObjectPointer(
            swapchain.createImageView(
                textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT)));
        _images.push_back(std::move(textureImage));
        _buffers.emplace_back(image._data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
        Buffer &addedBuffer = _buffers.back();
        addedBuffer.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        addedBuffer.fill(image._data.data());
        addedBuffer.bindMemory();
    }
}

void SceneTextures::loadSampler(const core::Texture &texture) {
    if (texture._sampler != nullptr) {
        const core::Sampler &sampler = *texture._sampler;
        const VkSamplerCreateInfo createInfo = sampler.generateVulkanCreateInfo();
        VkSampler textureSampler = VK_NULL_HANDLE;
        // todo process error!
        [[maybe_unused]] const VkResult samplerCreateResult = vkCreateSampler(_logicalDevice.getHandle(), &createInfo, nullptr, &textureSampler);
        _samplers.push_back(_logicalDevice.createObjectPointer(textureSampler));
    }
}

} // namespace avocado::vulkan

