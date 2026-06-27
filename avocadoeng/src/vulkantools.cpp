#include "vulkantools.hpp"

#include "config.hpp"

#include "vulkan/vkutils.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_syswm.h>

#include <vulkan/vulkan.h>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR // todo need this?
#include <vulkan/vulkan_win32.h>
#endif

using namespace std::literals::string_literals;

namespace avocado::vulkan {

void Vulkan::createInstance(const std::vector<std::string> &extensions,
    const std::vector<std::string> &layers, const VulkanInstanceInfo &vii) {
    DEFINE_VK_STRUCTURE(VkInstanceCreateInfo, instanceCreateInfo);

    DEFINE_VK_STRUCTURE(VkApplicationInfo, appInfo);
    appInfo.pApplicationName = vii.appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(vii.appMajorVersion, vii.appMinorVersion, vii.appPatchVersion);
    appInfo.pEngineName = avocado::core::Config::ENGINE_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(
       avocado::core::Config::ENGINE_MAJOR_VERSION
       , avocado::core::Config::ENGINE_MINOR_VERSION
       , avocado::core::Config::ENGINE_PATCH_VERSION);
    appInfo.apiVersion = VK_MAKE_API_VERSION(1, vii.apiMajorVersion, vii.apiMinorVersion, 0);
    instanceCreateInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> layerNamesCString(layers.size());
    for (size_t i = 0; i < layers.size(); ++i) {
        layerNamesCString[i] = layers[i].c_str();
    }
    if (!layers.empty())
        setLayers(instanceCreateInfo, layerNamesCString);

    std::vector<const char*> extensionNamesCString(extensions.size());
    for (size_t i = 0; i < extensions.size(); ++i) {
        extensionNamesCString[i] = extensions[i].c_str();
    }
    if (!extensions.empty())
        vulkan::setExtensions(instanceCreateInfo, extensionNamesCString);

    VkInstance instance = VK_NULL_HANDLE;
    CALL_VULKAN_AND_RETURN(vkCreateInstance, &instanceCreateInfo, nullptr, &instance);
    _instance.reset(instance);
}

// todo change prototype.
std::vector<std::string> Vulkan::getInstanceExtensions() const {
    unsigned int count = 0;
    CALL_VULKAN(vkEnumerateInstanceExtensionProperties, nullptr, &count, nullptr);

    std::vector<std::string> instanceExtensions;
    if (count > 0) {
        instanceExtensions.resize(count);
        std::vector<VkExtensionProperties> dataToFill(count);
        CALL_VULKAN_AND_RETURN_VALUE(instanceExtensions, vkEnumerateInstanceExtensionProperties, nullptr, &count, dataToFill.data());
        for (size_t i = 0; i < instanceExtensions.size(); ++i)
            instanceExtensions[i] = dataToFill[i].extensionName;
    }

    return instanceExtensions;
}


std::vector<std::string> Vulkan::getExtensionNamesForSDLSurface(SDL_Window *window) {
    std::vector<std::string> result;
    unsigned int extensionCount = 0;
    const SDL_bool getExtensionsCountResult = SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    if (SDL_FALSE == getExtensionsCountResult) {
        LOG_ERROR("Can't get SDL surface extensions count");
        return result;
    }

    if (extensionCount > 0) {
        result.resize(extensionCount);
        std::vector<const char*> dataToFill(extensionCount, nullptr);
        const SDL_bool extensionRetrievingResult = SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, dataToFill.data());
        if (SDL_FALSE == extensionRetrievingResult) {
            LOG_ERROR("Can't get SDL extensions' names");
            return result;
        }

        for (size_t i = 0; i < extensionCount; ++i)
            result[i] = dataToFill[i];
    }

    return result;
}

std::vector<VkLayerProperties> Vulkan::getLayerProperties() const {
    unsigned int count = 0;

    std::vector<VkLayerProperties> layerProps;
    CALL_VULKAN_AND_RETURN_VALUE(layerProps, vkEnumerateInstanceLayerProperties, &count, nullptr);

    if (count > 0) {
        layerProps.resize(count);
        CALL_VULKAN(vkEnumerateInstanceLayerProperties, &count, layerProps.data());
    }

    return layerProps;
}

std::vector<PhysicalDevice> Vulkan::getPhysicalDevices() {
    std::vector<VkPhysicalDevice> _result;
    std::vector<PhysicalDevice> result;
    uint32_t deviceCount = 0;
    CALL_VULKAN_AND_RETURN_VALUE(result, vkEnumeratePhysicalDevices, _instance.get(), &deviceCount, nullptr);

    if (deviceCount > 0) {
        _result.resize(deviceCount);
        result.resize(deviceCount);
        CALL_VULKAN_AND_RETURN_VALUE(result, vkEnumeratePhysicalDevices, _instance.get(), &deviceCount, _result.data());

        for (size_t i = 0; i < _result.size(); ++i)
            result[i] = PhysicalDevice(_result[i]);
    }

    return result;
}

bool Vulkan::areLayersSupported(const std::vector<std::string> &layerNames) const {
    std::vector<VkLayerProperties> layers = getLayerProperties();

    for (const std::string &layerName: layerNames) {
        const auto &foundIterator = std::find_if(layers.cbegin(), layers.cend(),
            [layerName](const VkLayerProperties &layerProps) {
                return (layerName == layerProps.layerName);
        });
        if (foundIterator == layers.cend()) {
            LOG_ERROR("Layer "s + layerName + " is not supported");
            return false;
        }
    }

    return true;
}

Surface Vulkan::createSurface(SDL_Window *window, PhysicalDevice &physicalDevice) {
    SDL_version sdlVersion; SDL_GetVersion(&sdlVersion);
    SDL_SysWMinfo wmInfo; wmInfo.version = sdlVersion;
    SDL_bool result = SDL_GetWindowWMInfo(window, &wmInfo);
    if (result != SDL_TRUE) {
        LOG_ERROR("SDL_GetWindowWMInfo returned SDL_FALSE");
        return Surface(VK_NULL_HANDLE, _instance.get(), physicalDevice);
    }

    VkSurfaceKHR surface;
    result = SDL_Vulkan_CreateSurface(window, _instance.get(), &surface);
    if (result != SDL_TRUE) {
        LOG_ERROR("SDL_Vulkan_CreateSurface returned SDL_FALSE");
        return Surface(VK_NULL_HANDLE, _instance.get(), physicalDevice);
    }

    return Surface(surface, _instance.get(), physicalDevice);
}


} // namespace avocado::vulkan.

