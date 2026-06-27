#include "surface.hpp"

#include "physicaldevice.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>

using namespace std::string_literals;

namespace avocado::vulkan {

Surface::Surface(VkSurfaceKHR surface, VkInstance instance, PhysicalDevice &physDev):
    _surface(surface), _instance(instance), _physicalDevice(physDev.getHandle()) {
}

Surface::~Surface() {
    if (_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
}

VkSurfaceKHR Surface::getHandle() noexcept {
    return _surface;
}

bool Surface::isValid() const noexcept {
    return (_surface != VK_NULL_HANDLE);
}

const std::vector<VkSurfaceFormatKHR> Surface::getSurfaceFormats() const {
    assert(_surface != VK_NULL_HANDLE && "Handle mustn't be null.");

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t formatCount = 0;

    CALL_VULKAN_AND_RETURN_VALUE(surfaceFormats, vkGetPhysicalDeviceSurfaceFormatsKHR, _physicalDevice, _surface, &formatCount, nullptr);

    if (formatCount > 0) {
        surfaceFormats.resize(formatCount);
        CALL_VULKAN_AND_RETURN_VALUE(surfaceFormats, vkGetPhysicalDeviceSurfaceFormatsKHR, _physicalDevice, _surface, &formatCount, surfaceFormats.data());
    }

    return surfaceFormats;

}

const std::vector<VkPresentModeKHR> Surface::getPresentModes() const {
    assert(_surface != VK_NULL_HANDLE && "Handle mustn't be null.");

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount = 0;
    CALL_VULKAN_AND_RETURN_VALUE(presentModes, vkGetPhysicalDeviceSurfacePresentModesKHR, _physicalDevice, _surface, &presentModeCount, nullptr);

    if (presentModeCount > 0) {
        presentModes.resize(presentModeCount);
        CALL_VULKAN_AND_RETURN_VALUE(presentModes, vkGetPhysicalDeviceSurfacePresentModesKHR, _physicalDevice, _surface, &presentModeCount, presentModes.data());
    }

    return presentModes;
}

VkExtent2D Surface::getCapabilities(SDL_Window *sdlWindow) noexcept {
    assert(_surface != VK_NULL_HANDLE && "Handle mustn't be null.");

    VkExtent2D extent{};

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    CALL_VULKAN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, _physicalDevice, _surface, &surfaceCapabilities);
    if (VK_SUCCESS == callResult) {
        extent = surfaceCapabilities.currentExtent;
        _minImageCount = surfaceCapabilities.minImageCount;
        _maxImageCount = surfaceCapabilities.maxImageCount;
        _minExtentH = surfaceCapabilities.minImageExtent.height;
        _maxExtentH = surfaceCapabilities.maxImageExtent.height;
        _minExtentW = surfaceCapabilities.minImageExtent.width;
        _maxExtentW = surfaceCapabilities.maxImageExtent.width;
        _currentTransform = surfaceCapabilities.currentTransform;
    } else {
        int width = 0, height = 0;
        SDL_Vulkan_GetDrawableSize(sdlWindow, &width, &height);
        extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    return extent;
}

VkSurfaceFormatKHR Surface::findFormat(VkFormat surfFormat, VkColorSpaceKHR colorSpace) const {
    assert(_surface != VK_NULL_HANDLE && "Handle mustn't be null.");

    VkSurfaceFormatKHR resultFormat{};
    const auto &surfaceFormats = getSurfaceFormats();
    if (surfaceFormats.empty())
        return resultFormat;

    const auto &findResult = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
        [surfFormat, colorSpace] (VkSurfaceFormatKHR surfaceFormat) {
        return (surfaceFormat.format == surfFormat && surfaceFormat.colorSpace == colorSpace);
    });
    if (findResult != surfaceFormats.cend())
        resultFormat = *findResult;

    return resultFormat;
}


const uint32_t Surface::getMinImageCount() const noexcept {
    return _minImageCount;
}

const uint32_t Surface::getMaxImageCount() const noexcept {
    return _maxImageCount;
}

const uint32_t Surface::getMinExtentH() const noexcept {
    return _minExtentH;
}

const uint32_t Surface::getMaxExtentH() const noexcept {
    return _maxExtentH;
}

const uint32_t Surface::getMinExtentW() const noexcept {
    return _minExtentW;
}

const uint32_t Surface::getMaxExtentW() const noexcept {
    return _maxExtentW;
}

const VkSurfaceTransformFlagBitsKHR Surface::getCurrentTransform() const noexcept {
    return _currentTransform;
}

}
