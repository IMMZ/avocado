#ifndef VULKANTOOLS_HPP
#define VULKANTOOLS_HPP

#include "core.hpp"
#include "errorstorage.hpp"

#include "vulkan/physicaldevice.hpp"
#include "vulkan/surface.hpp"

#include <vulkan/vulkan.h>

#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#define STRINGIFY_HELPER(str) #str
#define STRINGIFY(str) STRINGIFY_HELPER(str)

struct SDL_Window;

namespace avocado::vulkan::internal {

// Some useful type traits.
template <typename T>
struct IsVulkanCreateInfoType {
    static constexpr bool value = false;
};

template <>
struct IsVulkanCreateInfoType<VkInstanceCreateInfo> {
    static constexpr bool value = true;
};

template <>
struct IsVulkanCreateInfoType<VkDeviceCreateInfo> {
    static constexpr bool value = true;
};

template <typename T>
inline constexpr bool IsVulkanCreateInfoType_V = IsVulkanCreateInfoType<T>::value;

} // namespace avocado::vulkan::internal.


namespace avocado::vulkan {

struct VulkanInstanceInfo {
    const char *appName = nullptr;
    uint32_t appMajorVersion= 0;
    uint32_t appMinorVersion = 0;
    uint32_t appPatchVersion = 0;
    uint32_t apiMajorVersion = 0;
    uint32_t apiMinorVersion = 0;
};

class Vulkan: public avocado::core::ErrorStorage {
public:
    std::vector<std::string> getInstanceExtensions() const;

    std::vector<std::string> getExtensionNamesForSDLSurface(SDL_Window *window);
    std::vector<VkLayerProperties> getLayerProperties() const;
    std::vector<PhysicalDevice> getPhysicalDevices();
    Surface createSurface(SDL_Window *window, PhysicalDevice &physDev);

    /**
     * Checks if layers are supported.
     * Checking is done by comparing the absence of the input layers in the list, got by getLayerProperties().
     * @param layerNames Names of the layers which needed to be checked.
     * @retval true All input layers are supported.
     * @retval false All input layers are not supported.
     */
    bool areLayersSupported(const std::vector<std::string> &layerNames) const;

    void createInstance(const std::vector<std::string> &extensions, const std::vector<std::string> &layers, const VulkanInstanceInfo &vio);

private:
    VkInstance _instance = VK_NULL_HANDLE;
};

template <typename T>
inline void setExtensions(T &structure, const std::vector<const char*> &extensions) {
    static_assert(internal::IsVulkanCreateInfoType_V<T>, "Type must be Vulkan CreateInfo struct.");

    structure.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    structure.ppEnabledExtensionNames = extensions.data();
}

// todo such method is in PhysicalDevice. Extract from here. Search for 'ppEnabledExtension, -//- Layers'
template <typename T>
inline void setLayers(T &structure, const std::vector<const char*> &layers) {
    static_assert(internal::IsVulkanCreateInfoType_V<T>, "Type must be Vulkan CreateInfo struct.");

    structure.enabledLayerCount = static_cast<uint32_t>(layers.size());
    structure.ppEnabledLayerNames = layers.data();
}

} // namespace avocado::vulkan.

#endif

