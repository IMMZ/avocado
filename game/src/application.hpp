#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "vertex.hpp"

#include <vulkan/commandpool.hpp>
#include <vulkan/graphicspipeline.hpp>
#include <vulkan/swapchain.hpp>
#include <vulkan/vulkaninstance.hpp>

#include <SDL.h>

#include <../../third_party/tinygltf-release/tiny_gltf.h>

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
    avocado::vulkan::Swapchain createSwapchain(avocado::vulkan::Surface &surface, const VkSurfaceFormatKHR surfaceFormat, const VkExtent2D extent,
        const std::vector<avocado::vulkan::QueueFamily> &queueFamilies);
    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> createWindow();
    inline bool init() { return (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0); }

    avocado::vulkan::GraphicsPipelineBuilder preparePipeline(const VkExtent2D extent,
        const std::vector<VkDescriptorSetLayout> &layouts, const std::vector<VkViewport> &viewPorts,
        const std::vector<VkRect2D> &scissors);

    std::tuple<avocado::vulkan::Image, avocado::vulkan::ImageViewPtr, avocado::vulkan::SamplerPtr> loadTexture(avocado::vulkan::Swapchain &swapChain,
        avocado::vulkan::CommandPool &commandPool, avocado::vulkan::Queue &graphicsQueue, const tinygltf::Image &image);

    tinygltf::Model _model;
    avocado::vulkan::VulkanInstance _vulkanInstance;
    avocado::vulkan::PhysicalDevice _physicalDevice;
    avocado::vulkan::LogicalDevice _logicalDevice = avocado::vulkan::LogicalDevice::createNullDevice();
    static constexpr size_t FRAMES_IN_FLIGHT = 2;
};

#endif // APPLICATION_HPP
