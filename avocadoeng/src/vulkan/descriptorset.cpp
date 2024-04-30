#include "descriptorset.hpp"

#include "buffer.hpp"
#include "logicaldevice.hpp"
#include "structuretypes.hpp"
#include "vulkan_core.h"

namespace avocado::vulkan {

DescriptorSet::DescriptorSet (LogicalDevice &logicalDevice, const uint32_t count):
    _descriptorPool(logicalDevice.createDescriptorPool(count))
    , _device(logicalDevice) {
}

DescriptorSet::~DescriptorSet() {
    utils::makeUniqueContainer(_layouts);
    for (VkDescriptorSetLayout &layout: _layouts)
        vkDestroyDescriptorSetLayout(_device.getHandle(), layout, nullptr);
}

void DescriptorSet::addLayoutBinding(const uint32_t binding, const VkDescriptorType descriptorType, const uint32_t descriptorCount
    , const VkShaderStageFlags stageFlags, const VkSampler *immutableSamplers) {
    _layoutBindings.push_back(VkDescriptorSetLayoutBinding{binding, descriptorType, descriptorCount, stageFlags, immutableSamplers});
}

void DescriptorSet::createLayouts(const uint32_t copiesCount) {
    VkDescriptorSetLayoutCreateInfo layoutInfo{}; FILL_S_TYPE(layoutInfo);
    layoutInfo.bindingCount = _layoutBindings.size();
    layoutInfo.pBindings = _layoutBindings.data();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    const VkResult result = vkCreateDescriptorSetLayout(_device.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout);
    setHasError(result != VK_SUCCESS);
    if (!hasError()) {
        _layouts.insert(_layouts.cend(), copiesCount, descriptorSetLayout);
    } else {
        setErrorMessage("vkCreateDescriptorSetLayout returned "s + getVkResultString(result));
    }
}

const std::vector<VkDescriptorSetLayout>& DescriptorSet::getLayouts() const {
    return _layouts;
}

void DescriptorSet::allocate(const uint32_t setCount) {
    VkDescriptorSetAllocateInfo allocInfo{}; FILL_S_TYPE(allocInfo);
    allocInfo.descriptorPool = _descriptorPool.get();
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = _layouts.data();

    _descriptorSets.resize(setCount);
    const VkResult result = vkAllocateDescriptorSets(_device.getHandle(), &allocInfo, _descriptorSets.data());
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkAllocateDescriptorSets returned "s + getVkResultString(result));
}

void DescriptorSet::addBufferDescriptorWrite(const size_t descriptorSetIndex, const uint32_t dstBinding, const uint32_t dstArrayElement, const uint32_t descriptorCount) {
    VkWriteDescriptorSet bufferDescriptorWrite{}; FILL_S_TYPE(bufferDescriptorWrite);
    bufferDescriptorWrite.dstSet = _descriptorSets[descriptorSetIndex];
    bufferDescriptorWrite.dstBinding = dstBinding;
    bufferDescriptorWrite.dstArrayElement = dstArrayElement;
    bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferDescriptorWrite.descriptorCount = descriptorCount;
    bufferDescriptorWrite.pBufferInfo = _bufferInfos.data();
    _descriptorSetWrites.push_back(std::move(bufferDescriptorWrite));
}

void DescriptorSet::addBufferInfo(Buffer &buffer, const VkDeviceSize offset, const VkDeviceSize range) {
    _bufferInfos.emplace_back(VkDescriptorBufferInfo{buffer.getHandle(), offset, range});
}

void DescriptorSet::addImageDescriptorWrite(const size_t descriptorSetIndex, const uint32_t dstBinding, const uint32_t dstArrayElement, const uint32_t descriptorCount) {
    VkWriteDescriptorSet imageDescriptorWrite{}; FILL_S_TYPE(imageDescriptorWrite);
    imageDescriptorWrite.dstSet = _descriptorSets[descriptorSetIndex];
    imageDescriptorWrite.dstBinding = dstBinding;
    imageDescriptorWrite.dstArrayElement = dstArrayElement;
    imageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageDescriptorWrite.descriptorCount = descriptorCount;
    imageDescriptorWrite.pImageInfo = _imageInfos.data();
    _descriptorSetWrites.push_back(std::move(imageDescriptorWrite));
}

void DescriptorSet::addImageInfo(ImageViewPtr &imageView, SamplerPtr &sampler, const VkImageLayout imageLayout) {
    _imageInfos.emplace_back(VkDescriptorImageInfo{sampler.get(), imageView.get(), imageLayout});
}

void DescriptorSet::updateBuffer(const size_t bufferIndex, Buffer &buffer) {
    _bufferInfos[bufferIndex].buffer = buffer.getHandle();
}

void DescriptorSet::updateWriteDestinationSetIndex(const size_t writeIndex, const size_t destinationSetIndex) {
    _descriptorSetWrites[writeIndex].dstSet = _descriptorSets[destinationSetIndex];
}

void DescriptorSet::update() {
    vkUpdateDescriptorSets(_device.getHandle(), _descriptorSetWrites.size(), _descriptorSetWrites.data(), 0, nullptr);
}

void DescriptorSet::clear() {
    _descriptorSetWrites.clear();
    _bufferInfos.clear();
    _imageInfos.clear();
}

const VkDescriptorSet& DescriptorSet::getSet(const size_t index) const {
    return _descriptorSets[index];
}

} // namespace avocado::vulkan.

