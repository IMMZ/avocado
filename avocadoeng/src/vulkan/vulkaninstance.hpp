#ifndef VULKANTOOLS_HPP
#define VULKANTOOLS_HPP

#include "vulkan/physicaldevice.hpp"
#include "vulkan/pointertypes.hpp"
#include "vulkan/surface.hpp"

#include <vulkan/vulkan.h>

#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct SDL_Window;

namespace avocado::vulkan {

struct VulkanInstanceInfo {
  const char *appName = nullptr;
  uint32_t appMajorVersion = 0;
  uint32_t appMinorVersion = 0;
  uint32_t appPatchVersion = 0;
  uint32_t apiMajorVersion = 0;
  uint32_t apiMinorVersion = 0;
};

class VulkanInstance {
public:
  std::vector<std::string> getExtensionNamesForSDLSurface(SDL_Window *window);
  std::vector<VkLayerProperties> getLayerProperties() const;
  std::vector<PhysicalDevice> getPhysicalDevices();
  Surface createSurface(SDL_Window *window, PhysicalDevice &physDev);

  /**
   * @brief Checks if layers are supported.
   *
   * Checking is done by comparing the absence of the input layers in the list,
   * got by getLayerProperties().
   *
   * @param layerNames Names of the layers which needed to be checked.
   *
   * @retval true All input layers are supported.
   * @retval false All input layers are not supported.
   */
  bool areLayersSupported(const std::vector<std::string> &layerNames) const;

  void createInstance(const std::vector<std::string> &extensions,
                      const std::vector<std::string> &layers,
                      const VulkanInstanceInfo &vio);

private:
  InstancePtr _instance = VK_NULL_HANDLE;
};

} // namespace avocado::vulkan.

#endif
