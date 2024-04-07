#ifndef AVOCADO_VULKAN_SWAPCHAIN
#define AVOCADO_VULKAN_SWAPCHAIN

#include "pointertypes.hpp"
#include "types.hpp"

#include "../errorstorage.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class LogicalDevice;
class PhysicalDevice;
class Queue;
class Surface;

class Swapchain final: public core::ErrorStorage {
public:
    explicit Swapchain(LogicalDevice &logicalDevice);
    ~Swapchain();

    Swapchain(Swapchain &&other);
    Swapchain& operator=(Swapchain &&other);

    VkSwapchainKHR getHandle() noexcept;
    bool isValid() const noexcept;

    void create(Surface &surface, VkSurfaceFormatKHR surfaceFormat, VkExtent2D extent, const uint32_t minImageCount, const std::vector<QueueFamily> &queueFamilies) noexcept;
    void getImages();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkPhysicalDevice physDevice, const VkImageTiling tiling, const VkFormatFeatureFlags features);
    void createDepthImage(const uint32_t imgW, const uint32_t imgH, PhysicalDevice &physDevice, const VkMemoryPropertyFlags memoryFlags);
    VkImage getDepthImage() noexcept;
    VkImageView createImageView(VkImage image, VkFormat format, const VkImageAspectFlags aspectFlags);
    void createImageViews(VkSurfaceFormatKHR surfaceFormat);
    void createDepthImageView(VkSurfaceFormatKHR surfaceFormat);
    void createFramebuffers(VkRenderPass renderPass, VkExtent2D extent);
    uint32_t acquireNextImage(VkSemaphore semaphore) noexcept;
    VkFramebuffer getFramebuffer(size_t index);

private:
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;
    VkImage _depthImage;
    VkImageView _depthImageView;
    VkDeviceMemory _depthImageMemory;
    std::vector<VkFramebuffer> _framebuffers;
    VkDevice _device = VK_NULL_HANDLE;
    SwapchainKHRPtr _swapchain;
};

}

#endif

