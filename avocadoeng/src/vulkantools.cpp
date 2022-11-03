#include "vulkantools.hpp"

#include "config.hpp"

#include "vulkan/vkutils.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_syswm.h>

#include <vulkan/vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR // todo need this?
#include <vulkan/vulkan_win32.h>

#define VKRESULT_TO_STRING(VKRES) #VKRES // todo need this?

using namespace std::literals::string_literals;

namespace avocado::vulkan {

void Vulkan::createInstance(const std::vector<std::string> &extensions,
    const std::vector<std::string> &layers, const VulkanInstanceInfo &vii) {
    auto instanceCreateInfo = createStruct<VkInstanceCreateInfo>();
    
    auto appInfo = createStruct<VkApplicationInfo>();
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

    const VkResult createInstanceResult = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
    setHasError(createInstanceResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateInstance returned "s + getVkResultString(createInstanceResult));
    }
}

// todo change prototype.
std::vector<std::string> Vulkan::getInstanceExtensions() const {
    unsigned int count = 0;
    VkResult callResult = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    setHasError(callResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkEnumerateInstanceExtensionProperties returned "s + getVkResultString(callResult));
    }

    std::vector<std::string> result;
    if (count > 0) {
        result.resize(count);
        std::vector<VkExtensionProperties> dataToFill(count);
        callResult = vkEnumerateInstanceExtensionProperties(nullptr, &count, dataToFill.data());
        setHasError(callResult != VK_SUCCESS);
        if (!hasError()) {
            for (size_t i = 0; i < result.size(); ++i) {
                result[i] = dataToFill[i].extensionName;
            }
        } else {
            setErrorMessage("vkEnumerateInstanceExtensionProperties returned "s + getVkResultString(callResult));
        }
    }

    return result;
}


std::vector<std::string> Vulkan::getExtensionNamesForSDLSurface(SDL_Window *window) {
    std::vector<std::string> result;
    unsigned int extensionCount = 0;
    const SDL_bool getExtensionsCountResult = SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    setHasError(getExtensionsCountResult == SDL_FALSE);
    if (hasError()) {
        setErrorMessage("Can't get SDL surface extensions count");
        return result;
    }

    if (extensionCount > 0) {
        result.resize(extensionCount);
        std::vector<const char*> dataToFill(extensionCount, nullptr);
        const SDL_bool extensionRetrievingResult = SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, dataToFill.data());
        setHasError(extensionRetrievingResult == SDL_FALSE);
        if (!hasError()) {
            for (size_t i = 0; i < extensionCount; ++i) {
                result[i] = dataToFill[i];
            }
        } else {
            setErrorMessage("Can't get SDL extensions' names");
        }
    }

    return result;
}

std::vector<VkLayerProperties> Vulkan::getLayerProperties() const {
    unsigned int count = 0;
    
    std::vector<VkLayerProperties> layerProps;
    const VkResult firstCall = vkEnumerateInstanceLayerProperties(&count, nullptr);
    setHasError(firstCall != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkEnumerateInstanceLayerProperties returned "s + getVkResultString(firstCall));
        return layerProps;
    }

    if (count > 0) {
        layerProps.resize(count);
        const VkResult secondCall = vkEnumerateInstanceLayerProperties(&count, layerProps.data());
        setHasError(secondCall != VK_SUCCESS);
        if (hasError())
            setErrorMessage("2nd vkEnumerateInstanceLayerProperties returned "s + getVkResultString(secondCall));
    }

    return layerProps;
}

std::vector<PhysicalDevice> Vulkan::getPhysicalDevices() {
    std::vector<VkPhysicalDevice> _result;
    std::vector<PhysicalDevice> result;
    uint32_t deviceCount = 0;
    const VkResult enumerateResult1 = vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    if (enumerateResult1 != VK_SUCCESS) {
        setHasError(true);
        if (hasError())
            setErrorMessage("vkEnumeratePhysicalDevices returned "s + getVkResultString(enumerateResult1));
        return result;
    }

    if (deviceCount > 0) {
        _result.resize(deviceCount);
        result.resize(deviceCount);
        const VkResult enumerateResult2 = vkEnumeratePhysicalDevices(_instance, &deviceCount, _result.data());
        if (enumerateResult2 != VK_SUCCESS) {
            setHasError(true);
            if (hasError())
                setErrorMessage("vkEnumeratePhysicalDevices returned "s + getVkResultString(enumerateResult2));
            return result;
        }

        for (size_t i = 0; i < _result.size(); ++i) {
            result[i] = PhysicalDevice(_result[i]);
        }
    }

    return result;
}

bool Vulkan::areLayersSupported(const std::vector<std::string> &layerNames) const {
    std::vector<VkLayerProperties> layers = getLayerProperties();
    if (hasError())
        return false;

    for (const std::string &layerName: layerNames) {
        const auto &foundIterator = std::find_if(layers.cbegin(), layers.cend(),
            [layerName](const VkLayerProperties &layerProps) {
                return (layerName == layerProps.layerName);
        });
        if (foundIterator == layers.cend()) {
            setErrorMessage("Layer "s + layerName + " is not supported");
            return false;
        }
    }

    return true;
}


Surface Vulkan::createSurface(SDL_Window *window, PhysicalDevice &physicalDevice) {
    SDL_version sdlVersion; SDL_GetVersion(&sdlVersion);
    SDL_SysWMinfo wmInfo; wmInfo.version = sdlVersion;
    SDL_bool result = SDL_GetWindowWMInfo(window, &wmInfo);
    setHasError(result != SDL_TRUE);
    if (hasError()) {
        setErrorMessage("SDL_GetWindowWMInfo returned SDL_FALSE");
        return Surface(VK_NULL_HANDLE, _instance, physicalDevice);
    }

    VkSurfaceKHR surface;
    result = SDL_Vulkan_CreateSurface(window, _instance, &surface);
    setHasError(result != SDL_TRUE);
    if (hasError()) {
        setErrorMessage("SDL_Vulkan_CreateSurface returned SDL_FALSE");
        return Surface(VK_NULL_HANDLE, _instance, physicalDevice);
    }

    return Surface(surface, _instance, physicalDevice);
}


} // namespace avocado::vulkan.

