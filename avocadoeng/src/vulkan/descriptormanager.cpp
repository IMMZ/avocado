#include "descriptormanager.hpp"

#include "debugutils.hpp"
#include "logicaldevice.hpp"
#include "structuretypes.hpp"

#include "../config.hpp"
#include "../utils.hpp"

#include <algorithm>
#include <array>

/* Descriptor management scheme:
 *
 * Set 0
 *  binding 0: UBO, 3d matrices (model, view, projection);
 *
 * Set 1
 *  binding 0: reserved for future (fox xxxFactor variables)
 *  binding 1: image sampler array, baseColorTextures
 */
namespace {
    enum class SetIndex: size_t {
        Matrices
        , Materials

        , Count
    };

    constexpr uint32_t UBOBindingIndex = 0;

    enum class MaterialBindingIndex: uint32_t {
        Reserved
        , BaseColorTextures
    };
}

namespace avocado::vulkan {

DescriptorManager::DescriptorManager(LogicalDevice &device):
    _device(device),
    _descriptorPool(_device.createObjectPointer<VkDescriptorPool>(VK_NULL_HANDLE)) {
    createPool();
    allocateSets();
}

DescriptorManager::~DescriptorManager() {
    for (VkDescriptorSetLayout layout: _layouts)
        vkDestroyDescriptorSetLayout(_device.getHandle(), layout, nullptr);
}

void DescriptorManager::createPool() {
    constexpr std::array<VkDescriptorPoolSize, 1> descriptorPoolSizes {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, avocado::core::Config::MAX_BINDLESS_RESOURCES}
    };

    DEFINE_VK_STRUCTURE(VkDescriptorPoolCreateInfo, dPoolCI);
    dPoolCI.poolSizeCount = descriptorPoolSizes.size();
    dPoolCI.pPoolSizes = descriptorPoolSizes.data();

    // maxSets is related to total sets for ALL the sets.
    dPoolCI.maxSets = avocado::utils::enumToInteger(SetIndex::Count) + 1000;
    dPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    vkCreateDescriptorPool(_device.getHandle(), &dPoolCI, nullptr, &descriptorPool);
    _descriptorPool = _device.createObjectPointer<VkDescriptorPool>(descriptorPool);
}

// todo Do we plan to use it more than once? Maybe rename to => setUBO(...)
void DescriptorManager::addBuffer(Buffer &buffer, const uint32_t range, const uint32_t offset) {
    _bufferInfos.push_back(VkDescriptorBufferInfo{buffer.getHandle(), offset, range});
}

void DescriptorManager::addImage(ImageViewPtr &view, SamplerSharedPtr &sampler, const VkImageLayout layout) {
    assert(!_sets.empty() && "No sets available. Are they allocated?");

    _imageInfos.push_back(VkDescriptorImageInfo{sampler.get(), view.get(), layout});

}

void DescriptorManager::update() {
    DEFINE_VK_STRUCTURE(VkWriteDescriptorSet, imageDescriptorWrite);
    imageDescriptorWrite.dstSet = _sets[avocado::utils::enumToInteger(SetIndex::Materials)];
    imageDescriptorWrite.dstBinding = avocado::utils::enumToInteger(MaterialBindingIndex::BaseColorTextures);
    imageDescriptorWrite.dstArrayElement = 0;
    imageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageDescriptorWrite.descriptorCount = _imageInfos.size();
    imageDescriptorWrite.pImageInfo = _imageInfos.data();

    DEFINE_VK_STRUCTURE(VkWriteDescriptorSet, bufferDescriptorWrite);
    bufferDescriptorWrite.dstSet = _sets[avocado::utils::enumToInteger(SetIndex::Matrices)];
    bufferDescriptorWrite.dstBinding = UBOBindingIndex;
    bufferDescriptorWrite.dstArrayElement = 0;
    bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferDescriptorWrite.descriptorCount = _bufferInfos.size();
    bufferDescriptorWrite.pBufferInfo = _bufferInfos.data();

    const std::array<VkWriteDescriptorSet, 2> writes { imageDescriptorWrite, bufferDescriptorWrite };
    vkUpdateDescriptorSets(_device.getHandle(), writes.size(), writes.data(), 0, nullptr);
}

void DescriptorManager::createLayouts() {
    _layouts.clear();

    DEFINE_VK_STRUCTURE(VkDescriptorSetLayoutBindingFlagsCreateInfoEXT, bindingFlagsCI);
    bindingFlagsCI.bindingCount = 1;
    constexpr VkDescriptorBindingFlags bindingFlags =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
        | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
        | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    bindingFlagsCI.pBindingFlags = &bindingFlags;

    // Create matrices layout.
    constexpr VkDescriptorSetLayoutBinding bindingForMatrices {
        UBOBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 /* descriptorCount */,
        VK_SHADER_STAGE_VERTEX_BIT, nullptr };

    DEFINE_VK_STRUCTURE(VkDescriptorSetLayoutCreateInfo, layoutCI);
    layoutCI.bindingCount = 1;
    layoutCI.pBindings = &bindingForMatrices;
    layoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layoutCI.pNext = &bindingFlagsCI;

    CALL_VULKAN(vkCreateDescriptorSetLayout, _device.getHandle(), &layoutCI, nullptr, &_matricesSetLayout);

    // Create materials layout.
    constexpr VkDescriptorSetLayoutBinding bindingForBaseColorTextures {
        1 /* binding */, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          avocado::core::Config::MAX_BINDLESS_RESOURCES, VK_SHADER_STAGE_ALL, nullptr };
    layoutCI.pBindings = &bindingForBaseColorTextures;
    layoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    CALL_VULKAN_AND_DEFINE_VARIABLE(creationResult, vkCreateDescriptorSetLayout, _device.getHandle(), &layoutCI, nullptr, &_materialsSetLayout);

    _layouts.push_back(_matricesSetLayout);
    _layouts.push_back(_materialsSetLayout);
}

void DescriptorManager::allocateSets() {
    createLayouts();

    _sets.resize(avocado::utils::enumToInteger(SetIndex::Count));
    std::fill(_sets.begin(), _sets.end(), VK_NULL_HANDLE);

    DEFINE_VK_STRUCTURE(VkDescriptorSetVariableDescriptorCountAllocateInfoEXT, countInfo);
    countInfo.descriptorSetCount = 1;
    uint32_t descriptorCount = 1;
    countInfo.pDescriptorCounts = &descriptorCount;

    DEFINE_VK_STRUCTURE(VkDescriptorSetAllocateInfo, allocInfo);
    allocInfo.descriptorPool = _descriptorPool.get();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_matricesSetLayout;
    allocInfo.pNext = &countInfo;

    CALL_VULKAN(vkAllocateDescriptorSets, _device.getHandle(), &allocInfo, &_sets[avocado::utils::enumToInteger(SetIndex::Matrices)]);

    allocInfo.pSetLayouts = &_materialsSetLayout;
    descriptorCount = avocado::core::Config::MAX_BINDLESS_RESOURCES - 1;

    CALL_VULKAN_AND_DEFINE_VARIABLE(allocationResult, vkAllocateDescriptorSets, _device.getHandle(), &allocInfo, &_sets[avocado::utils::enumToInteger(SetIndex::Materials)]);

    auto debugUtilsPtr = _device.createDebugUtils();
    debugUtilsPtr->setObjectName(_sets[0], "Set with UBO");
    debugUtilsPtr->setObjectName(_sets[1], "Set with image sample array");
}

const std::vector<VkDescriptorSetLayout>& DescriptorManager::getLayouts() const {
    return _layouts;
}

const std::vector<VkDescriptorSet>& DescriptorManager::getSets() const {
    return _sets;
}

} // namespace avocado::vulkan
