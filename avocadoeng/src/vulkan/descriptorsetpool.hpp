#ifndef AVOCADO_VULKAN_DESCRIPTOR_SET
#define AVOCADO_VULKAN_DESCRIPTOR_SET

#include "../errorstorage.hpp"

#include "pointertypes.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace avocado::vulkan {

class Buffer;
class LogicalDevice;



class DescriptorSetPool: public core::ErrorStorage {
public:
    explicit DescriptorSetPool(LogicalDevice &logicalDevice, const uint32_t count);
    DescriptorSetPool(DescriptorSetPool &&other);
    ~DescriptorSetPool();

    DescriptorSetPool& operator=(DescriptorSetPool &&other);

    void addLayoutBinding(const VkDescriptorType descriptorType, const uint32_t descriptorCount
        , const VkShaderStageFlags stageFlags, const VkSampler *immutableSamplers = nullptr);
    void createLayouts(const uint32_t copiesCount = 1);
    const std::vector<VkDescriptorSetLayout>& getLayouts() const;
    void allocate(const uint32_t setCount);
    const VkDescriptorSet& getSet(const size_t index) const;
    size_t addBuffer(const uint32_t setIndex, const uint32_t binding, Buffer &buffer,
        const uint32_t range, const uint32_t offset = 0);
    size_t addImage(const uint32_t setIndex, const uint32_t binding, ImageViewPtr &imageView, SamplerPtr &sampler, const VkImageLayout imageLayout);

    void updateBuffer(const size_t writeIndex, Buffer &buffer);
    void updateWriteDestinationSetIndex(const size_t writeIndex, const size_t destinationSetIndex);

    void update();
    void clear();

private:
    std::vector<VkDescriptorSetLayoutBinding> _layoutBindings;
    std::vector<VkDescriptorSetLayout> _layouts;
    std::vector<VkDescriptorSet> _descriptorSets;
    std::vector<VkWriteDescriptorSet> _descriptorSetWrites;
    std::vector<VkDescriptorBufferInfo> _bufferInfos;
    std::vector<VkDescriptorImageInfo> _imageInfos;
    DescriptorPoolPtr _descriptorPool;
    LogicalDevice &_device;
};

} // namespace avocado::vulkan.

#endif
