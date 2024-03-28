#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "vulkantools.hpp" // todo get rid of this header file

#include <vulkan/graphicspipeline.hpp>

#include <SDL.h>

#include <memory>

class Application {
public:
    int run();

private:
    void createInstance(SDL_Window &window, const std::vector<std::string> &instanceLayers);
    void createPhysicalDevice();
    avocado::vulkan::SamplerPtr createSampler();
    avocado::vulkan::Swapchain createSwapchain(avocado::vulkan::Surface &surface, const VkSurfaceFormatKHR surfaceFormat, const VkExtent2D extent,
        const std::vector<avocado::vulkan::QueueFamily> &queueFamilies);
    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> createWindow();
    inline bool init() {
        return (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0);
    }

    avocado::vulkan::GraphicsPipelineBuilder preparePipeline(const VkExtent2D extent,
        std::vector<VkDescriptorSetLayout> &layouts, const std::vector<VkViewport> &viewPorts,
        const std::vector<VkRect2D> &scissors);

    void updateDescriptorSets(std::vector<avocado::vulkan::Buffer*> &uniformBuffers, const VkDeviceSize range, std::vector<VkDescriptorSet> &descriptorSets,
        avocado::vulkan::ImageViewPtr &textureImageView, avocado::vulkan::SamplerPtr &textureSampler);

    avocado::vulkan::Vulkan _vulkan;
    avocado::vulkan::PhysicalDevice _physicalDevice;
    avocado::vulkan::LogicalDevice _logicalDevice;
    static constexpr size_t FRAMES_IN_FLIGHT = 2;
};

#endif // APPLICATION_HPP
