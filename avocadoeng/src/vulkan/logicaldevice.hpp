#ifndef AVOCADO_VULKAN_LOGICAL_DEVICE
#define AVOCADO_VULKAN_LOGICAL_DEVICE

#include "queue.hpp"
#include "pointertypes.hpp"
#include "types.hpp"

#include "../errorstorage.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <limits>
#include <functional>
#include <memory>
#include <vector>

using namespace std::string_literals;

namespace avocado::vulkan {

class Buffer;
class DebugUtils;
class PhysicalDevice;

class LogicalDevice: public avocado::core::ErrorStorage {
public:

    VkDevice getHandle() noexcept;
    bool isValid() const noexcept;

    void updateDescriptorSet(const std::vector<VkWriteDescriptorSet> &descriptorWriteSets) noexcept;
    VkDescriptorBufferInfo createDescriptorBufferInfo(Buffer &buffer, const size_t bufferSize) noexcept;
    DescriptorPoolPtr createDescriptorPool(const size_t descriptorCount);
    RenderPassPtr createRenderPass(VkFormat format, VkFormat depthFormat);
    Queue getGraphicsQueue(const uint32_t index) noexcept;
    Queue getPresentQueue(const uint32_t index) noexcept;
    Queue getTransferQueue(const uint32_t index) noexcept;

    std::unique_ptr<DebugUtils> createDebugUtils();

    FencePtr createFence() noexcept;
    void waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout = std::numeric_limits<uint64_t>::max()) noexcept;
    void resetFences(const std::vector<VkFence> &fences) noexcept;
    SemaphorePtr createSemaphore() noexcept;

    // todo Looks like physicalDevice param is extra. It should be as a class member.
    SamplerPtr createSampler(PhysicalDevice &physicalDevice);
    SamplerPtr createSampler(PhysicalDevice &physicalDevice, const VkSamplerCreateInfo &createInfo);

    template <typename T>
    ObjectPtr<T> createObjectPointer(T objectHandle) {
        return ObjectPtr<T>(objectHandle, ObjectDeleter<T>(*this));
    }

    template <typename T>
    AllocatedObjectPtr<T> createAllocatedObjectPointer(T objectHandle) {
        return AllocatedObjectPtr<T>(objectHandle, AllocatedObjectDeleter<T>(*this));
    }

    void waitIdle() noexcept;

    [[nodiscard]] static inline LogicalDevice createNullDevice() {
        return LogicalDevice(VK_NULL_HANDLE);
    }

private:
    friend class PhysicalDevice;

    explicit LogicalDevice();
    explicit LogicalDevice(VkDevice dev);
    void setQueueFamilies(const QueueFamily graphicsQF, const QueueFamily presentQF, const QueueFamily transferQF) noexcept;

    DevicePtr _dev;
    QueueFamily _graphicsQueueFamily = 0, _presentQueueFamily = 0, _transferQueueFamily = 0;
};

} // namespace vulkan.

#endif

