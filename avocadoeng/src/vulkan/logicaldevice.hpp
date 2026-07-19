#ifndef AVOCADO_VULKAN_LOGICAL_DEVICE
#define AVOCADO_VULKAN_LOGICAL_DEVICE

#include "queue.hpp"
#include "pointertypes.hpp"
#include "structuretypes.hpp"
#include "types.hpp"

#include "../callvulkan.hpp"

#include <memory>
#include <vector>

using namespace std::string_literals;

namespace avocado::vulkan {

class Buffer;
class PhysicalDevice;

class LogicalDevice {
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

    FencePtr createFence() noexcept;
    void waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout = std::numeric_limits<uint64_t>::max()) noexcept;
    void resetFences(const std::vector<VkFence> &fences) noexcept;
    SemaphorePtr createSemaphore() noexcept;

    template <typename T>
    ObjectPtr<T> createObjectPointer(T objectHandle) {
        return ObjectPtr<T>(objectHandle, ObjectDeleter<T>(*this));
    }

    template <typename T>
    ObjectSharedPtr<T> createObjectSharedPointer(T objectHandle) {
        return ObjectSharedPtr<T>(objectHandle, ObjectDeleter<T>(*this));
    }

    template <typename T>
    AllocatedObjectPtr<T> createAllocatedObjectPointer(T objectHandle) {
        return AllocatedObjectPtr<T>(objectHandle, AllocatedObjectDeleter<T>(*this));
    }

    void waitIdle() noexcept;

    [[nodiscard]] static inline LogicalDevice createNullDevice() {
        return LogicalDevice(VK_NULL_HANDLE);
    }

    // Debug features are extracted to separate file.
    #include "devicedebugutils.hpp"

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

