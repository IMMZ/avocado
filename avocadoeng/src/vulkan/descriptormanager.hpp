#ifndef AVOCADO_VULKAN_DESCRIPTOR_MANAGER
#define AVOCADO_VULKAN_DESCRIPTOR_MANAGER

#include "buffer.hpp"
#include "image.hpp"
#include "pointertypes.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class Buffer;
class LogicalDevice;

class DescriptorManager {
public:
    explicit DescriptorManager(LogicalDevice &device);
    ~DescriptorManager();

    void addBuffer(Buffer &buffer, const uint32_t range, const uint32_t offset);
    void addImage(ImageViewPtr &view, SamplerSharedPtr &sampler, const VkImageLayout layout);
    void update();
    const std::vector<VkDescriptorSetLayout>& getLayouts() const;
    const std::vector<VkDescriptorSet>& getSets() const;

private:
    void createPool();
    void setup();
    void createLayouts();
    void allocateSets();

    LogicalDevice &_device;
    DescriptorPoolPtr _descriptorPool;
    std::vector<VkDescriptorSetLayout> _layouts;
    std::vector<VkDescriptorSet> _sets;
    std::vector<VkWriteDescriptorSet> _descriptorWrites;
    std::vector<VkDescriptorBufferInfo> _bufferInfos;
    std::vector<VkDescriptorImageInfo> _imageInfos;
    VkDescriptorSetLayout _matricesSetLayout = VK_NULL_HANDLE,
        _materialsSetLayout = VK_NULL_HANDLE;
};

} // namespace avocado::vulkan

#endif
