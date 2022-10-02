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

constexpr const char* getVkResultString(const VkResult vkres) {
    #define PROCESS_CODE(code) case VK_ ##code: { return #code; }
    switch (vkres) {
        PROCESS_CODE(SUCCESS)
        PROCESS_CODE(NOT_READY)
        PROCESS_CODE(TIMEOUT)
        PROCESS_CODE(EVENT_SET)
        PROCESS_CODE(EVENT_RESET)
        PROCESS_CODE(INCOMPLETE)
        PROCESS_CODE(ERROR_OUT_OF_HOST_MEMORY)
        PROCESS_CODE(ERROR_OUT_OF_DEVICE_MEMORY)
        PROCESS_CODE(ERROR_INITIALIZATION_FAILED)
        PROCESS_CODE(ERROR_DEVICE_LOST)
        PROCESS_CODE(ERROR_MEMORY_MAP_FAILED)
        PROCESS_CODE(ERROR_LAYER_NOT_PRESENT)
        PROCESS_CODE(ERROR_EXTENSION_NOT_PRESENT)
        PROCESS_CODE(ERROR_FEATURE_NOT_PRESENT)
        PROCESS_CODE(ERROR_INCOMPATIBLE_DRIVER)
        PROCESS_CODE(ERROR_TOO_MANY_OBJECTS)
        PROCESS_CODE(ERROR_FORMAT_NOT_SUPPORTED)
        PROCESS_CODE(ERROR_FRAGMENTED_POOL)
        PROCESS_CODE(ERROR_UNKNOWN)
        PROCESS_CODE(ERROR_OUT_OF_POOL_MEMORY)
        PROCESS_CODE(ERROR_INVALID_EXTERNAL_HANDLE)
        PROCESS_CODE(ERROR_FRAGMENTATION)
        PROCESS_CODE(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
        PROCESS_CODE(PIPELINE_COMPILE_REQUIRED)
        PROCESS_CODE(ERROR_SURFACE_LOST_KHR)
        PROCESS_CODE(ERROR_NATIVE_WINDOW_IN_USE_KHR)
        PROCESS_CODE(SUBOPTIMAL_KHR)
        PROCESS_CODE(ERROR_OUT_OF_DATE_KHR)
        PROCESS_CODE(ERROR_INCOMPATIBLE_DISPLAY_KHR)
        PROCESS_CODE(ERROR_VALIDATION_FAILED_EXT)
        PROCESS_CODE(ERROR_INVALID_SHADER_NV)
        PROCESS_CODE(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
        PROCESS_CODE(ERROR_NOT_PERMITTED_KHR)
        PROCESS_CODE(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
        PROCESS_CODE(THREAD_IDLE_KHR)
        PROCESS_CODE(THREAD_DONE_KHR)
        PROCESS_CODE(OPERATION_DEFERRED_KHR)
        PROCESS_CODE(OPERATION_NOT_DEFERRED_KHR)
        PROCESS_CODE(RESULT_MAX_ENUM)
        default: { assert(false); return "UNKNOWN_ERROR"; }
    }

    // All code here is unreachable.
    return "UNREACHABLE_ERROR";
    #undef PROCESS_CODE
}

template<typename ...Args, typename Fn = VkResult(*)(Args...)>
VkResult callVulkanFn(Fn vulkanFn, Args... args) {
    const VkResult res = vulkanFn(args...);
    if (res != VK_SUCCESS)
        throw VulkanError(std::string(STRINGIFY(vulkanFn)) + " returned " + getVkResultString(res));
    return res;
}

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
    uint32_t apiVersion = 0;
};

class Vulkan: public avocado::core::ErrorStorage {
public:
    core::Result getInstanceExtensions(std::vector<VkExtensionProperties> &extensions);

    bool getExtensionNamesForSDLSurface(SDL_Window *window, std::vector<const char*> &result);
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
    bool areLayersSupported(const std::vector<const char *> &layerNames);

    void createInstance(const std::vector<const char*> &extensions, const std::vector<const char*> &layers, const VulkanInstanceInfo &vio);

private:
    VkInstance _instance = VK_NULL_HANDLE;
};

template <typename T>
inline void setExtensions(T &structure, const std::vector<const char*> &extensions) {
    static_assert(internal::IsVulkanCreateInfoType_V<T>, "Type must be Vulkan CreateInfo struct.");

    structure.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    structure.ppEnabledExtensionNames = extensions.data();
}

template <typename T>
inline void setLayers(T &structure, const std::vector<const char*> &layers) {
    static_assert(internal::IsVulkanCreateInfoType_V<T>, "Type must be Vulkan CreateInfo struct.");

    structure.enabledLayerCount = static_cast<uint32_t>(layers.size());
    structure.ppEnabledLayerNames = layers.data();
}

} // namespace avocado::vulkan.

#endif

