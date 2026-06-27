#include "descriptorsetpool.hpp"

#include "buffer.hpp"
#include "logicaldevice.hpp"
#include "structuretypes.hpp"

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

DescriptorSetPool::DescriptorSetPool (LogicalDevice &logicalDevice, const uint32_t count):
    _descriptorPool(logicalDevice.createDescriptorPool(count))
    , _device(logicalDevice) {
}

DescriptorSetPool::DescriptorSetPool(DescriptorSetPool &&other):
    _descriptorPool(std::move(other._descriptorPool)),
    _device(other._device) {
    _layoutBindings = std::move(other._layoutBindings);
    _layouts = std::move(other._layouts);
    _descriptorSets = std::move(other._descriptorSets);
    _descriptorSetWrites = std::move(other._descriptorSetWrites);
    _bufferInfos = std::move(other._bufferInfos);
    _imageInfos = std::move(other._imageInfos);
}

DescriptorSetPool::~DescriptorSetPool() {
    utils::makeUniqueContainer(_layouts);
    for (VkDescriptorSetLayout &layout: _layouts)
        vkDestroyDescriptorSetLayout(_device.getHandle(), layout, nullptr);
}

void DescriptorSetPool::addLayoutBinding(const VkDescriptorType descriptorType, const uint32_t descriptorCount
    , const VkShaderStageFlags stageFlags, const VkSampler *immutableSamplers) {
    _layoutBindings.push_back(VkDescriptorSetLayoutBinding{
        static_cast<uint32_t>(_layoutBindings.size()), descriptorType, descriptorCount, stageFlags, immutableSamplers});
}

void DescriptorSetPool::createLayouts(const uint32_t copiesCount) {
    DEFINE_VK_STRUCTURE(VkDescriptorSetLayoutCreateInfo, layoutInfo);
    layoutInfo.bindingCount = _layoutBindings.size();
    layoutInfo.pBindings = _layoutBindings.data();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    CALL_VULKAN_AND_RETURN(vkCreateDescriptorSetLayout, _device.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout);
    _layouts.insert(_layouts.cend(), copiesCount, descriptorSetLayout);
}

const std::vector<VkDescriptorSetLayout>& DescriptorSetPool::getLayouts() const {
    return _layouts;
}

void DescriptorSetPool::allocate(const uint32_t setCount) {
    // At first create layouts.
    DEFINE_VK_STRUCTURE(VkDescriptorSetLayoutCreateInfo, layoutInfo);
    layoutInfo.bindingCount = _layoutBindings.size();
    layoutInfo.pBindings = _layoutBindings.data();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    CALL_VULKAN_AND_RETURN(vkCreateDescriptorSetLayout, _device.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout);
    _layouts.insert(_layouts.cend(), setCount, descriptorSetLayout);

    DEFINE_VK_STRUCTURE(VkDescriptorSetAllocateInfo, allocInfo);
    allocInfo.descriptorPool = _descriptorPool.get();
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = _layouts.data();

    _descriptorSets.resize(setCount);
    CALL_VULKAN_AND_DEFINE_VARIABLE(allocationResult, vkAllocateDescriptorSets, _device.getHandle(), &allocInfo, _descriptorSets.data());
}

void DescriptorSetPool::updateBuffer(const size_t bufferIndex, Buffer &buffer) {
    _bufferInfos[bufferIndex].buffer = buffer.getHandle();
}

void DescriptorSetPool::updateWriteDestinationSetIndex(const size_t writeIndex, const size_t destinationSetIndex) {
    _descriptorSetWrites[writeIndex].dstSet = _descriptorSets[destinationSetIndex];
}

void DescriptorSetPool::update() {
    size_t bufIndex = 0, imgIndex = 0;
    for (VkWriteDescriptorSet &writeDescriptor: _descriptorSetWrites) {
        if (writeDescriptor.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            writeDescriptor.pImageInfo = &_imageInfos[imgIndex];
            imgIndex++;
        } else {
            writeDescriptor.pBufferInfo = &_bufferInfos[bufIndex];
            bufIndex++;
        }
    }

    vkUpdateDescriptorSets(_device.getHandle(), _descriptorSetWrites.size(), _descriptorSetWrites.data(), 0, nullptr);
}

void DescriptorSetPool::clear() {
    _descriptorSetWrites.clear();
    _bufferInfos.clear();
    _imageInfos.clear();
}

const VkDescriptorSet& DescriptorSetPool::getSet(const size_t index) const {
    return _descriptorSets[index];
}

size_t DescriptorSetPool::addBuffer(const uint32_t setIndex, const uint32_t binding, Buffer &buffer,
    const uint32_t range, const uint32_t offset) {
    assert(!_descriptorSets.empty() && "Are descriptor sets allocated?");
    assert(setIndex < _descriptorSets.size() && "Descriptor set index must be < setsCount");

    _bufferInfos.push_back(VkDescriptorBufferInfo{buffer.getHandle(), offset, range});

    DEFINE_VK_STRUCTURE(VkWriteDescriptorSet, bufferDescriptorWrite);
    bufferDescriptorWrite.dstSet = _descriptorSets[setIndex];
    bufferDescriptorWrite.dstBinding = binding;
    bufferDescriptorWrite.dstArrayElement = 0;
    bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferDescriptorWrite.descriptorCount = 1;
    _descriptorSetWrites.push_back(std::move(bufferDescriptorWrite));
    return _descriptorSetWrites.size();
}

size_t DescriptorSetPool::addImage(const uint32_t setIndex, const uint32_t binding, ImageViewPtr &imageView, SamplerPtr &sampler, const VkImageLayout imageLayout) {
    assert(!_descriptorSets.empty() && "Are descriptor sets allocated?");
    assert(setIndex < _descriptorSets.size() && "Descriptor set index must be < setsCount");

    _imageInfos.push_back(VkDescriptorImageInfo{sampler.get(), imageView.get(), imageLayout});

    DEFINE_VK_STRUCTURE(VkWriteDescriptorSet, imageDescriptorWrite);
    imageDescriptorWrite.dstSet = _descriptorSets[setIndex];
    imageDescriptorWrite.dstBinding = binding;
    imageDescriptorWrite.dstArrayElement = 0;
    imageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageDescriptorWrite.descriptorCount = 1;
    _descriptorSetWrites.push_back(std::move(imageDescriptorWrite));
    return _descriptorSetWrites.size();
}

} // namespace avocado::vulkan.

