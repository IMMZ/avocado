#ifndef AVOCADO_VULKAN_PHYSICAL_DEVICE 
#define AVOCADO_VULKAN_PHYSICAL_DEVICE

#include "../errorstorage.hpp"
#include "../utils.hpp"

#include "types.hpp"
#include "logicaldevice.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class Surface;

class PhysicalDevice final: public core::ErrorStorage {
public:
    MAKE_MOVABLE(PhysicalDevice);

    PhysicalDevice();
    explicit PhysicalDevice(VkPhysicalDevice device);

    VkPhysicalDevice getHandle();
    bool isValid() const;
    std::vector<std::string> getPhysicalDeviceExtensions() const;

    void getQueueFamilies(Surface &surface);
    QueueFamily getGraphicsQueueFamily() const;
    QueueFamily getPresentQueueFamily() const;

    LogicalDevice createLogicalDevice(
        const std::vector<uint32_t> &uniqueQueueFamilyIndices,
        const std::vector<std::string> &extensions,
        const std::vector<std::string> &instanceLayers,
        const uint32_t queueCount, const float queuePriority);
    bool areExtensionsSupported(const std::vector<std::string> &extNames) const;

private:
    NON_COPYABLE(PhysicalDevice);

    VkPhysicalDevice _device;
    QueueFamily _graphicsQueueFamily = std::numeric_limits<QueueFamily>::max(),
        _presentQueueFamily = std::numeric_limits<QueueFamily>::max();
};

}

#endif

