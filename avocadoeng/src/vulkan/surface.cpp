#include "surface.hpp"

#include "physicaldevice.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>

namespace avocado::vulkan {

Surface::Surface(VkSurfaceKHR surface, VkInstance instance, PhysicalDevice &physDev):
    _surface(surface), _instance(instance), _physicalDevice(physDev.getHandle()) {
}

Surface::~Surface() {
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
}

VkSurfaceKHR Surface::getHandle() {
    return _surface;
}

bool Surface::isValid() const {
    return (_surface != VK_NULL_HANDLE);
}

const std::vector<VkSurfaceFormatKHR> Surface::getSurfaceFormats() const {
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t formatCount = 0;
    const VkResult result1 = vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr);
    if (result1 != VK_SUCCESS) {
        setHasError(true);
        // todo fix
        // setErrorMessage("vkGetPhysicalDeviceSurfaceFormatsKHR() returned "s + internal::getVkResultString(result1));
        return surfaceFormats;
    }

    if (formatCount > 0) {
        surfaceFormats.resize(formatCount);
        const VkResult result2 = vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, surfaceFormats.data());
        if (result2 != VK_SUCCESS) {
            setHasError(true);
            // todo fix
            // setErrorMessage("vkGetPhysicalDeviceSurfaceFormatsKHR() returned "s + internal::getVkResultString(result2));
            return surfaceFormats;
        }
    }

    return surfaceFormats;

}

const std::vector<VkPresentModeKHR> Surface::getPresentModes() const {
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount = 0;
    const VkResult result1 = vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, nullptr);
    if (result1 != VK_SUCCESS) {
        setHasError(true);
        // todo fix
        //setErrorMessage("vkGetPhysicalDeviceSurfacePresentModesKHR() returned "s + internal::getVkResultString(result1));
        return presentModes;
    }

    if (presentModeCount > 0) {
        presentModes.resize(presentModeCount);
        const VkResult result2 = vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, presentModes.data());
        if (result2 != VK_SUCCESS) {
            setHasError(true);
            //todo fix
            //setErrorMessage("vkGetPhysicalDeviceSurfacePresentModesKHR() returned "s + internal::getVkResultString(result2));
            return presentModes;
        }
    }

    return presentModes; 
}

const uint32_t Surface::getPresentQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilies) const {
    VkBool32 presentSupport = VK_FALSE;
    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        const VkResult surfSupportResult = vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &presentSupport);
        if (surfSupportResult != VK_SUCCESS) {
            setHasError(true);
            // todo fix
            // setErrorMessage("vkGetPhysicalDeviceSurfaceSupportKHR returned "s + internal::getVkResultString(surfSupportResult));
            return std::numeric_limits<uint32_t>::max();
        }

        if (presentSupport == VK_TRUE)
            return static_cast<uint32_t>(i);
    }

    return std::numeric_limits<uint32_t>::max();
}

VkExtent2D Surface::getCapabilities(SDL_Window *sdlWindow) {
    VkExtent2D extent{};

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCapabilities);
    if (result == VK_SUCCESS) {
        extent = surfaceCapabilities.currentExtent;
        _minImageCount = surfaceCapabilities.minImageCount;
        _maxImageCount = surfaceCapabilities.maxImageCount;
        _minExtentH = surfaceCapabilities.minImageExtent.height;
        _maxExtentH = surfaceCapabilities.maxImageExtent.height;
        _minExtentW = surfaceCapabilities.minImageExtent.width;
        _maxExtentW = surfaceCapabilities.maxImageExtent.width;
        _currentTransform = surfaceCapabilities.currentTransform;
    } else {
        setHasError(true);
        // todo fix
        // setErrorMessage("vkGetPhysicalDeviceSurfaceCapabilitiesKHR returned"s + internal::getVkResultString(result));
        int width = 0, height = 0;
        SDL_Vulkan_GetDrawableSize(sdlWindow, &width, &height);
        extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    return extent;
}

VkSurfaceFormatKHR Surface::findFormat(SurfaceFormat sf, ColorSpace cs) const {
    VkSurfaceFormatKHR resultFormat{};
    const auto &surfaceFormats = getSurfaceFormats();
    if (hasError()) {
        setErrorMessage("Can't get surface formats: " + getErrorMessage());
        return resultFormat;
    }

    const auto &findResult = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
        [sf, cs] (VkSurfaceFormatKHR surfaceFormat) {
        return (surfaceFormat.format == static_cast<VkFormat>(sf) && surfaceFormat.colorSpace == static_cast<VkColorSpaceKHR>(cs));
    });

    if (findResult != surfaceFormats.cend()) {
        resultFormat = *findResult;
    }
    else {
        setHasError(true);
        setErrorMessage("No suitable surface found");
    }

    return resultFormat;
}


const uint32_t Surface::getMinImageCount() const {
    return _minImageCount;
}

const uint32_t Surface::getMaxImageCount() const {
    return _maxImageCount;
}

const uint32_t Surface::getMinExtentH() const {
    return _minExtentH;
}

const uint32_t Surface::getMaxExtentH() const {
    return _maxExtentH;
}

const uint32_t Surface::getMinExtentW() const {
    return _minExtentW;
}

const uint32_t Surface::getMaxExtentW() const {
    return _maxExtentW;
}

const VkSurfaceTransformFlagBitsKHR Surface::getCurrentTransform() const {
    return _currentTransform;
}

}
