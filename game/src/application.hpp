#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "vulkantools.hpp" // todo get rid of this header file

#include <vulkan/commandpool.hpp>
#include <vulkan/graphicspipeline.hpp>
#include <vulkan/swapchain.hpp>

#include <SDL.h>

#include <memory>

class Application {
public:
    int run();

    enum class CommandBufferIndex: size_t {
        CopyImageToBuffer = 2
        , TransferImageLayout1
        , TransferImageLayout2
    };

private:
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features);
    bool createDepthImage(const uint32_t imageW, const uint32_t imageH, avocado::vulkan::Swapchain &swapchain, avocado::vulkan::CommandPoolPtr &commandPool, avocado::vulkan::Queue &graphicsQueue);
    void createInstance(SDL_Window &window, const std::vector<std::string> &instanceLayers);
    void createPhysicalDevice();
    avocado::vulkan::SamplerPtr createSampler();
    avocado::vulkan::Swapchain createSwapchain(avocado::vulkan::Surface &surface, const VkSurfaceFormatKHR surfaceFormat, const VkExtent2D extent,
        const std::vector<avocado::vulkan::QueueFamily> &queueFamilies);
    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> createWindow();
    inline bool init() { return (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0); }

    avocado::vulkan::GraphicsPipelineBuilder preparePipeline(const VkExtent2D extent,
        const std::vector<VkDescriptorSetLayout> &layouts, const std::vector<VkViewport> &viewPorts,
        const std::vector<VkRect2D> &scissors);

    void transitionImageLayout(avocado::vulkan::CommandBuffer &commandBuffer, avocado::vulkan::Queue &queue, avocado::vulkan::Image &image, VkFormat format,
        VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageAspectFlags aspectFlags);

    avocado::vulkan::Vulkan _vulkan;
    avocado::vulkan::PhysicalDevice _physicalDevice;
    avocado::vulkan::LogicalDevice _logicalDevice = avocado::vulkan::LogicalDevice::createNullDevice();
    static constexpr size_t FRAMES_IN_FLIGHT = 2;
};

#endif // APPLICATION_HPP
