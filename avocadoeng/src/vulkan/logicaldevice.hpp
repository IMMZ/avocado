#ifndef AVOCADO_VULKAN_LOGICAL_DEVICE
#define AVOCADO_VULKAN_LOGICAL_DEVICE

#include "commandbuffer.hpp"
#include "queue.hpp"
#include "pointertypes.hpp"
#include "types.hpp"

#include "../errorstorage.hpp"
#include "vulkan_core.h"

#include <vulkan/vulkan.h>

#include <limits>
#include <functional>
#include <memory>
#include <vector>

using namespace std::string_literals;

namespace avocado::vulkan {

class DebugUtils;
class PhysicalDevice;

class LogicalDevice: public avocado::core::ErrorStorage {
public:
    // todo do we really need it public? Only physical device can create this.
    explicit LogicalDevice();
    explicit LogicalDevice(VkDevice dev);

    VkDevice getHandle() noexcept;
    bool isValid() const noexcept;

    VkDescriptorSetLayoutBinding createLayoutBinding(const uint32_t bindingNumber, const VkDescriptorType type,
        const uint32_t descriptorCount, const VkShaderStageFlags flags, const std::vector<VkSampler> &samplers = {}) noexcept;

    VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    void updateDescriptorSet(const std::vector<VkWriteDescriptorSet> &descriptorWriteSets) noexcept;
    VkDescriptorBufferInfo createDescriptorBufferInfo(Buffer &buffer, const size_t bufferSize) noexcept;
    std::pair<VkDescriptorSet, VkWriteDescriptorSet> createWriteDescriptorSet(
        VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorBufferInfo &descriptorBufferInfo);
    VkDescriptorPool createDescriptorPool(const size_t descriptorCount);
    RenderPassPtr createRenderPass(VkFormat format);
    Queue getGraphicsQueue(const uint32_t index) noexcept;
    Queue getPresentQueue(const uint32_t index) noexcept;
    Queue getTransferQueue(const uint32_t index) noexcept;

    std::unique_ptr<DebugUtils> createDebugUtils();

    // todo this is supposed to be used by PhysicalDevice, not straightly.
    void setQueueFamilies(const QueueFamily graphicsQF, const QueueFamily presentQF, const QueueFamily transferQF) noexcept;
    VkFence createFence() noexcept;
    void waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout = std::numeric_limits<uint64_t>::max()) noexcept;
    void resetFences(const std::vector<VkFence> &fences) noexcept;
    VkSemaphore createSemaphore() noexcept;

    SamplerPtr createSampler(PhysicalDevice &physicalDevice);

    template <typename T>
    ObjectPtr<T> createObjectPointer(T objectHandle) {
        return ObjectPtr<T>(objectHandle, ObjectDeleter<T>(*this));
    }

    template <typename T>
    AllocatedObjectPtr<T> createAllocatedObjectPointer(T objectHandle) {
        return AllocatedObjectPtr<T>(objectHandle, AllocatedObjectDeleter<T>(*this));
    }

    VkCommandPool createCommandPool(const VkCommandPoolCreateFlags flags, const QueueFamily queueFamilyIndex) noexcept;
    std::vector<CommandBuffer> allocateCommandBuffers(const uint32_t count, VkCommandPool cmdPool, const VkCommandBufferLevel bufLevel);

    void waitIdle() noexcept;

private:
    DevicePtr _dev;
    QueueFamily _graphicsQueueFamily = 0, _presentQueueFamily = 0, _transferQueueFamily = 0;
};

} // namespace vulkan.

#endif

