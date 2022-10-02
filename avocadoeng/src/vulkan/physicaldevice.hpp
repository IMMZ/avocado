#ifndef AVOCADO_VULKAN_PHYSICAL_DEVICE 
#define AVOCADO_VULKAN_PHYSICAL_DEVICE

#include "../errorstorage.hpp"
#include "../utils.hpp"

#include "logicaldevice.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class PhysicalDevice final: public core::ErrorStorage {
public:
    MAKE_MOVABLE(PhysicalDevice);

    PhysicalDevice();
    explicit PhysicalDevice(VkPhysicalDevice device);

    VkPhysicalDevice getHandle();
    bool isValid() const;

    std::vector<VkQueueFamilyProperties> getQueueFamilies() const;

    std::vector<std::string> getPhysicalDeviceExtensions() const;
    uint32_t getGraphicsQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilies) const;
    LogicalDevice createLogicalDevice(
        const std::vector<uint32_t> &uniqueQueueFamilyIndices,
        const std::vector<const char*> &extensions,
        const std::vector<const char*> &instanceLayers,
        const uint32_t queueCount, const float queuePriority);
    bool areExtensionsSupported(const std::vector<const char*> &extNames) const;

private:
    NON_COPYABLE(PhysicalDevice);

    VkPhysicalDevice _device;
};

}

#endif

