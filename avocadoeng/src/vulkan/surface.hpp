#ifndef AVOCADO_VULKAN_SURFACE
#define AVOCADO_VULKAN_SURFACE

#include "format.hpp"

#include "../errorstorage.hpp"
#include "../utils.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

struct SDL_Window;

namespace avocado::vulkan {

class PhysicalDevice;

class Surface final: public core::ErrorStorage {
public:
    NON_COPYABLE(Surface);
    MAKE_MOVABLE(Surface);

    Surface(VkSurfaceKHR surface, VkInstance instance, PhysicalDevice &physicalDevice);
    ~Surface();

    VkSurfaceKHR getHandle() noexcept;
    bool isValid() const noexcept;

    const std::vector<VkPresentModeKHR> getPresentModes() const;
    VkExtent2D getCapabilities(SDL_Window *sdlWindow) noexcept;
    VkSurfaceFormatKHR findFormat(VkFormat sf, VkColorSpaceKHR cs) const;
    const uint32_t getMinImageCount() const noexcept;
    const uint32_t getMaxImageCount() const noexcept;
    const uint32_t getMinExtentH() const noexcept;
    const uint32_t getMaxExtentH() const noexcept;
    const uint32_t getMinExtentW() const noexcept;
    const uint32_t getMaxExtentW() const noexcept;
    const VkSurfaceTransformFlagBitsKHR getCurrentTransform() const noexcept;

private:
    const std::vector<VkSurfaceFormatKHR> getSurfaceFormats() const;

    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkInstance _instance = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkSurfaceTransformFlagBitsKHR _currentTransform = static_cast<VkSurfaceTransformFlagBitsKHR >(0);
    uint32_t _minImageCount = 0, _maxImageCount = 0;
    uint32_t _minExtentH = 0, _maxExtentH = 0
        , _minExtentW = 0, _maxExtentW = 0;
};

}

#endif

