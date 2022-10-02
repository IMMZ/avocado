#include "vulkantools.hpp"

#include "config.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_syswm.h>

#include <Windows.h>

#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>

#define VKRESULT_TO_STRING(VKRES) #VKRES // todo need this?

using namespace std::literals::string_literals;

namespace avocado::vulkan {

void Vulkan::createInstance(const std::vector<const char*> &extensions,
    const std::vector<const char*> &layers, const VulkanInstanceInfo &vii) {
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = vii.appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(vii.appMajorVersion, vii.appMinorVersion, vii.appPatchVersion);
    appInfo.pEngineName = avocado::core::Config::ENGINE_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(
       avocado::core::Config::ENGINE_MAJOR_VERSION
       , avocado::core::Config::ENGINE_MINOR_VERSION 
       , avocado::core::Config::ENGINE_PATCH_VERSION);
    appInfo.apiVersion = VK_API_VERSION_1_2; // todo make it possible to specify outside of this place (on client code);
    instanceCreateInfo.pApplicationInfo = &appInfo;

    if (!layers.empty())
        setLayers(instanceCreateInfo, layers);

    if (!extensions.empty())
        vulkan::setExtensions(instanceCreateInfo, extensions);

    const VkResult createInstanceResult = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
    setHasError(createInstanceResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateInstance returned "s + internal::getVkResultString(createInstanceResult));
    }
}

core::Result getInstanceExtensions(std::vector<VkExtensionProperties> &extensions) {
    unsigned int count = 0;

    if (vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) != VK_SUCCESS)
        return core::Result::Error;

    if (count > 0) {
        extensions.resize(count);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()) == VK_SUCCESS)
            return core::Result::Ok;

        return core::Result::Error;
    }

    return core::Result::Ok;
}


bool Vulkan::getExtensionNamesForSDLSurface(SDL_Window *window, std::vector<const char*> &result) {
    unsigned int extensionCount = 0;
    const SDL_bool getExtensionsCountResult = SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    if (getExtensionsCountResult == SDL_FALSE) {
        setErrorMessage("Can't get SDL surface extensions count");
        return false;
    }

    if (extensionCount > 0) {
        result.resize(extensionCount, nullptr);
        const SDL_bool extensionRetrievingResult = SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, result.data());
        if (extensionRetrievingResult == SDL_FALSE)
            setErrorMessage("Can't get SDL extensions' names");
        return (extensionRetrievingResult == SDL_TRUE);
    }

    return true; 
}

std::vector<VkLayerProperties> Vulkan::getLayerProperties() const {
    unsigned int count = 0;
    
    std::vector<VkLayerProperties> layerProps;
    const VkResult firstCall = vkEnumerateInstanceLayerProperties(&count, nullptr);
    setHasError(firstCall != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkEnumerateInstanceLayerProperties returned "s);
        return layerProps;
    }

    if (count > 0) {
        layerProps.resize(count);
        const VkResult secondCall = vkEnumerateInstanceLayerProperties(&count, layerProps.data());
        setHasError(secondCall != VK_SUCCESS);
        if (hasError())
            setErrorMessage("2nd vkEnumerateInstanceLayerProperties returned "s);
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
        setErrorMessage("vkEnumeratePhysicalDevices returned "s + internal::getVkResultString(enumerateResult1));
        return result;
    }

    if (deviceCount > 0) {
        _result.resize(deviceCount);
        result.resize(deviceCount);
        const VkResult enumerateResult2 = vkEnumeratePhysicalDevices(_instance, &deviceCount, _result.data());
        if (enumerateResult2 != VK_SUCCESS) {
            setHasError(true);
            setErrorMessage("vkEnumeratePhysicalDevices returned "s + internal::getVkResultString(enumerateResult2));
            return result;
        }

        for (size_t i = 0; i < _result.size(); ++i) {
            result[i] = PhysicalDevice(_result[i]);
        }
    }

    return result;
}

bool Vulkan::areLayersSupported(const std::vector<const char*> &layerNames /* todo change to vector<string> */) {
    std::vector<VkLayerProperties> layers = getLayerProperties();
    if (hasError())
        return false;

    for (const char * const layerName: layerNames) {
        const auto &foundIterator = std::find_if(layers.cbegin(), layers.cend(),
            [layerName](const VkLayerProperties &layerProps) {
                return (strcmp(layerName, layerProps.layerName) == 0);
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

    if (SDL_GetWindowWMInfo(window, &wmInfo) != SDL_TRUE) {
        setHasError(true);
        setErrorMessage("SDL_GetWindowWMInfo returned SDL_FALSE");
        return Surface(VK_NULL_HANDLE, _instance, physicalDevice);
    }

    VkWin32SurfaceCreateInfoKHR win32surfaceCreateInfo{};
    win32surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32surfaceCreateInfo.hwnd = wmInfo.info.win.window;
    win32surfaceCreateInfo.hinstance = wmInfo.info.win.hinstance;
   
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(window, _instance, &surface) != SDL_TRUE) {
        setHasError(true);
        setErrorMessage("SDL_Vulkan_CreateSurface returned SDL_FALSE");
        return Surface(VK_NULL_HANDLE, _instance, physicalDevice);
    }

    return Surface(surface, _instance, physicalDevice);
}


} // namespace avocado::vulkan.

