#ifndef AVOCADO_VULKAN_LOGICAL_DEVICE
#define AVOCADO_VULKAN_LOGICAL_DEVICE

#include "commandbuffer.hpp"
#include "queue.hpp"
#include "objectdeleter.hpp"
#include "types.hpp"

#include "../errorstorage.hpp"
#include "vulkan_core.h"

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <vector>

using namespace std::string_literals;

namespace avocado::vulkan {

class DebugUtils;
class LogicalDevice;

template <typename O>
void destroyObject(LogicalDevice &logicalDevice, O objHandle);

template <typename T>
struct Destroyer {
    Destroyer(LogicalDevice &logicalDevice):
        _logicalDevice(logicalDevice){}

    void operator()(T objHandle) {
        destroyObject<T>(_logicalDevice, objHandle);
    }

    LogicalDevice &_logicalDevice;
};

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
    VkDescriptorPool createDescriptorPool();

    Queue getGraphicsQueue(const uint32_t index) noexcept;
    Queue getPresentQueue(const uint32_t index) noexcept;
    Queue getTransferQueue(const uint32_t index) noexcept;

    std::unique_ptr<DebugUtils> createDebugUtils();

    // todo this is supposed to be used by PhysicalDevice, not straightly.
    void setQueueFamilies(const QueueFamily graphicsQF, const QueueFamily presentQF, const QueueFamily transferQF) noexcept;

    VkFence createFence() noexcept;

    // todo make last arg with default value.
    void waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout) noexcept;
    void resetFences(const std::vector<VkFence> &fences) noexcept;
    VkSemaphore createSemaphore() noexcept;


    template <typename T>
    ObjectPtr<T> createObjectPointer(T objectHandle) {
        return ObjectPtr<T>(objectHandle, ObjectDeleter<T>(*this));
    }

    enum class CommandPoolCreationFlags {
        Transient = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        Reset = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        Protected = VK_COMMAND_POOL_CREATE_PROTECTED_BIT,
    };
    VkCommandPool createCommandPool(const CommandPoolCreationFlags flags, const QueueFamily queueFamilyIndex) noexcept;

    enum class CommandBufferLevel {
        Primary = VK_COMMAND_BUFFER_LEVEL_PRIMARY
        , Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY
    };
    std::vector<CommandBuffer> allocateCommandBuffers(const uint32_t count, VkCommandPool cmdPool, const CommandBufferLevel bufLevel);

    void waitIdle() noexcept;

private:
    using DeviceDeleter = decltype(std::bind(vkDestroyDevice, std::placeholders::_1, nullptr));
    using DevicePtr = std::unique_ptr<VkDevice_T, DeviceDeleter>;
    DevicePtr _dev;

    // Queue families. Needed to return queues.
    QueueFamily _graphicsQueueFamily = 0, _presentQueueFamily = 0, _transferQueueFamily = 0;

public:
    using RenderPassUniquePtr = std::unique_ptr<
        std::remove_pointer_t<VkRenderPass>
        , decltype(std::bind(vkDestroyRenderPass, _dev.get(), std::placeholders::_1, nullptr))>;
    RenderPassUniquePtr createRenderPass(VkFormat format);
};

} // namespace vulkan.

#endif

