#ifndef AVOCADO_VULKAN_SURFACE
#define AVOCADO_VULKAN_SURFACE

#include "format.hpp"

#include "../errorstorage.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

struct SDL_Window;

namespace avocado::vulkan {

class PhysicalDevice;

class Surface final: public core::ErrorStorage {
public:
    enum class ColorSpace {
        SRGBNonlinearKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        DisplayP3NonlinearExt = VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
        ExtendedSRGBLinearExt = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
        DisplayP3LinearExt = VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
        DCIP3NonlinearExt = VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
        BT709LinearExt = VK_COLOR_SPACE_BT709_LINEAR_EXT,
        BT709NonlinearExt = VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
        BT2020LinearExt = VK_COLOR_SPACE_BT2020_LINEAR_EXT,
        HDR10ST2084Ext = VK_COLOR_SPACE_HDR10_ST2084_EXT,
        DolbyvisionExt = VK_COLOR_SPACE_DOLBYVISION_EXT,
        HDR10HLGExt = VK_COLOR_SPACE_HDR10_HLG_EXT,
        AdobeRGBLinearExt = VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,
        AdobeRGBNonlinearExt = VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT,
        PassThroughExt = VK_COLOR_SPACE_PASS_THROUGH_EXT,
        ExtendedSRGBNonlinearExt = VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT,
        DisplayNativeAMD = VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,
        DCIP3LinearExt = VK_COLOR_SPACE_DCI_P3_LINEAR_EXT
    };

    Surface(VkSurfaceKHR surface, VkInstance instance, PhysicalDevice &physicalDevice);
    ~Surface();

    VkSurfaceKHR getHandle();
    bool isValid() const;

    const std::vector<VkPresentModeKHR> getPresentModes() const;
    VkExtent2D getCapabilities(SDL_Window *sdlWindow);
    VkSurfaceFormatKHR findFormat(Format sf, ColorSpace cs) const;
    const uint32_t getMinImageCount() const;
    const uint32_t getMaxImageCount() const;
    const uint32_t getMinExtentH() const; 
    const uint32_t getMaxExtentH() const; 
    const uint32_t getMinExtentW() const; 
    const uint32_t getMaxExtentW() const; 
    const VkSurfaceTransformFlagBitsKHR getCurrentTransform() const;

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

