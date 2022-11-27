#include "gameconfig.hpp"

#include "vulkan/clipping.hpp"
#include "vulkan/colorattachment.hpp"
#include "vulkan/commandbuffer.hpp"
#include "vulkan/logicaldevice.hpp"
#include "vulkan/graphicspipeline.hpp"
#include "vulkan/graphicsqueue.hpp"
#include "vulkan/queue.hpp"
#include "vulkan/surface.hpp"
#include "vulkan/swapchain.hpp"
#include "vulkan/vkutils.hpp"
#include "vulkantools.hpp" // todo get rid of this header file

#include "utils.hpp"

#include <core.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_image.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

int SDL_main(int argc, char ** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "Can't init SDL." << std::endl;
        return 1;
    }

    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> sdlWindow(SDL_CreateWindow(
        Config::GAME_NAME,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN), SDL_DestroyWindow);

    if (sdlWindow == nullptr) {
        std::cerr << "Can't create window." << std::endl;
        return 1;
    }

    // Set window icon.
    const std::string &iconPath = std::filesystem::current_path().string() + "/icon.png";
    SDL_Surface * const iconSurface = IMG_Load(iconPath.c_str());
    if (iconSurface != nullptr)
        SDL_SetWindowIcon(sdlWindow.get(), iconSurface);

    avocado::vulkan::Vulkan vlk;
    const std::vector<std::string> instanceLayers {"VK_LAYER_KHRONOS_validation"};
    const bool areLayersSupported = vlk.areLayersSupported(instanceLayers);
    if (!areLayersSupported) {
        std::cerr << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<std::string> instanceExtensions = vlk.getExtensionNamesForSDLSurface(sdlWindow.get());
    if constexpr (avocado::core::isDebugBuild())
        instanceExtensions.push_back("VK_EXT_debug_utils");

    if (vlk.hasError()) {
        std::cerr << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    constexpr avocado::vulkan::VulkanInstanceInfo vulkanInfo {
        Config::GAME_NAME,
        0, 1, 0, // App version.
        1, 2 // Vulkan API version.
    };

    vlk.createInstance(instanceExtensions, instanceLayers, vulkanInfo);
    if (vlk.hasError()) {
        std::cerr << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::PhysicalDevice> physicalDevices = vlk.getPhysicalDevices();
    if (vlk.hasError()) {
        std::cout << "Can't get physical devices: " << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    if (physicalDevices.empty()) {
        std::cout << "No physical devices found." << std::endl;
        return 1;
    }

    avocado::vulkan::PhysicalDevice &physicalDevice = physicalDevices.front();
    const std::vector<std::string> physExtensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    physicalDevice.areExtensionsSupported(physExtensions);
    if (physicalDevice.hasError()) {
        std::cerr << "Extensions error: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::Surface surface = vlk.createSurface(sdlWindow.get(), physicalDevice);
    if (vlk.hasError()) {
        std::cerr << "Can't create surface: " << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    physicalDevice.getQueueFamilies(surface);
    if (physicalDevice.hasError()) {
        std::cout << "Can't get queue families: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }
    const avocado::vulkan::QueueFamily graphicsQueueFamily = physicalDevice.getGraphicsQueueFamily();
    const avocado::vulkan::QueueFamily presentQueueFamily = physicalDevice.getPresentQueueFamily();

    if (surface.hasError()) {
        std::cout << "Can't get present queue family index: " << surface.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::QueueFamily> queueFamilies {graphicsQueueFamily, presentQueueFamily};
    avocado::utils::makeUniqueContainer(queueFamilies);

    avocado::vulkan::LogicalDevice logicalDevice = physicalDevice.createLogicalDevice(queueFamilies, physExtensions, instanceLayers, 1, 1.0f);
    if (physicalDevice.hasError()) {
        std::cerr << "Can't create logical device: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::PresentQueue presentQueue = logicalDevice.getPresentQueue(0);

    const bool isSwapChainSuppported = physicalDevice.areExtensionsSupported({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    if (!isSwapChainSuppported) {
        std::cerr << "Swapchain isn't supported. (" << vlk.getErrorMessage() << ")" << std::endl;
        return 1;
    }

    VkExtent2D extent = surface.getCapabilities(sdlWindow.get());

    // Clamp width and height to fit into capabilities.
    extent.width = std::clamp(extent.width, surface.getMinExtentW(), surface.getMaxExtentW());
    extent.height = std::clamp(extent.height, surface.getMinExtentH(), surface.getMaxExtentH());

    // Creating swapchain.
    uint32_t imageCount = surface.getMinImageCount() + 1;
    if (surface.getMaxImageCount() > 0 && imageCount > surface.getMaxImageCount())
        imageCount = surface.getMaxImageCount();

    const VkSurfaceFormatKHR surfaceFormat = surface.findFormat(
        avocado::vulkan::Surface::SurfaceFormat::B8G8R8A8SRGB,
        avocado::vulkan::Surface::ColorSpace::SRGBNonlinearKHR);

    if (surface.hasError()) {
        std::cerr << "Can't get surface format: " << surface.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::Swapchain swapChain(logicalDevice);
    swapChain.create(surface, surfaceFormat, extent, imageCount, queueFamilies);

    swapChain.getImages();
    if (swapChain.hasError()) {
        std::cout << "Can't get swapchain images: " << swapChain.getErrorMessage() << std::endl;
        return 1;
    }

    swapChain.createImageViews(surfaceFormat);
    if (swapChain.hasError()) {
        std::cout << "Can't get swapchain images: " << swapChain.getErrorMessage() << std::endl;
        return 1;
    }

    // todo check for errors.
    const std::vector<char> &fragBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.frag.spv");
    const std::vector<char> &vertBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.vert.spv");

    // todo replace to pipeline class.
    const VkPipelineShaderStageCreateInfo &fragShaderModule =
        logicalDevice.addShaderModule(fragBuf, avocado::vulkan::LogicalDevice::ShaderType::Fragment);
    const VkPipelineShaderStageCreateInfo &vertShaderModule =
        logicalDevice.addShaderModule(vertBuf, avocado::vulkan::LogicalDevice::ShaderType::Vertex);
    const std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCIs = {fragShaderModule, vertShaderModule};

    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder(logicalDevice.getHandle());
    pipelineBuilder.addDynamicStates({
        avocado::vulkan::GraphicsPipelineBuilder::DynamicState::Viewport
        , avocado::vulkan::GraphicsPipelineBuilder::DynamicState::Scissor});

    // Input assembly.
    pipelineBuilder.setPrimitiveTopology(
        avocado::vulkan::GraphicsPipelineBuilder::PrimitiveTopology::TriangleList);

    // Rasterizer.
    pipelineBuilder.setPolygonMode(
        avocado::vulkan::GraphicsPipelineBuilder::PolygonMode::Fill);
    pipelineBuilder.setCullMode(
        avocado::vulkan::GraphicsPipelineBuilder::CullMode::BackBit);
    pipelineBuilder.setFrontFace(
        avocado::vulkan::GraphicsPipelineBuilder::FrontFace::Clockwise);

    pipelineBuilder.setSampleCount(
        avocado::vulkan::GraphicsPipelineBuilder::SampleCount::_1Bit);

    // Color blending.
    avocado::vulkan::ColorAttachment colorAttachment;

    colorAttachment.setColorComponent(avocado::utils::enumBitwiseOr(avocado::vulkan::ColorAttachment::ColorComponent::R
        , avocado::vulkan::ColorAttachment::ColorComponent::G
        , avocado::vulkan::ColorAttachment::ColorComponent::B
        , avocado::vulkan::ColorAttachment::ColorComponent::A));
    colorAttachment.setSrcBlendFactor(
        avocado::vulkan::ColorAttachment::BlendFactor::One);
    colorAttachment.setDstBlendFactor(
        avocado::vulkan::ColorAttachment::BlendFactor::Zero);
    colorAttachment.setSrcAlphaBlendFactor(
        avocado::vulkan::ColorAttachment::BlendFactor::One);
    colorAttachment.setDstAlphaBlendFactor(
        avocado::vulkan::ColorAttachment::BlendFactor::Zero);
    colorAttachment.setColorBlendOperation(
        avocado::vulkan::ColorAttachment::BlendOperation::Add);
    colorAttachment.setAlphaBlendOperation(
        avocado::vulkan::ColorAttachment::BlendOperation::Add);
    pipelineBuilder.addColorAttachment(std::move(colorAttachment));

    pipelineBuilder.setLogicOperation(
        avocado::vulkan::GraphicsPipelineBuilder::LogicOperation::Copy);

    auto renderPassPtr = logicalDevice.createRenderPass(surfaceFormat.format);

    const std::vector<VkViewport> viewPorts { avocado::vulkan::Clipping::createViewport(0.f, 0.f, extent) };
    const std::vector<VkRect2D> scissors { avocado::vulkan::Clipping::createScissor(viewPorts.front()) };
    avocado::vulkan::GraphicsPipelineBuilder::PipelineUniquePtr grPip = pipelineBuilder.buildPipeline(pipelineShaderStageCIs, surfaceFormat.format, renderPassPtr.get(), viewPorts, scissors);

    if (grPip == nullptr) {
        std::cout << "INVALID PIPELINE" << std::endl;
        return 1;
    }
    VkPipeline graphicsPipeline = grPip.get();

    swapChain.createFramebuffers(renderPassPtr.get(), extent);

    VkCommandPool commandPool = logicalDevice.createCommandPool(avocado::vulkan::LogicalDevice::CommandPoolCreationFlags::Reset, graphicsQueueFamily);
    if (logicalDevice.hasError()) {
        std::cout << "Can't create command pool: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::CommandBuffer> cmdBuffers = logicalDevice.allocateCommandBuffers(1, commandPool, avocado::vulkan::LogicalDevice::CommandBufferLevel::Primary);
    if (logicalDevice.hasError()) {
        std::cout << "Can't allocate command buffers (" << logicalDevice.getErrorMessage() << ")." << std::endl;
        return 1;
    }

    avocado::vulkan::CommandBuffer commandBuffer = cmdBuffers.front();
    commandBuffer.begin();
    if (commandBuffer.hasError()) {
        std::cout << "Can't begin recording command buffer." << std::endl;
        return 1;
    }

    commandBuffer.beginRenderPass(swapChain, renderPassPtr.get(), extent, {0, 0}, 0);

    commandBuffer.bindPipeline(graphicsPipeline, avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
    commandBuffer.setViewports(viewPorts);
    commandBuffer.setScissors(scissors);
    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
    if (commandBuffer.hasError()) {
        std::cout << "Can't end command buffer." << std::endl;
        return 1;
    }

    // Synchronization objects.
    VkSemaphore imageAvailableSemaphore = logicalDevice.createSemaphore();
    if (logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    VkSemaphore renderFinishedSemaphore = logicalDevice.createSemaphore();
    if (logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<VkFence> fences {logicalDevice.createFence()};
    if (logicalDevice.hasError()) {
        std::cout << "Can't create fence: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    // Main event loop.
    SDL_Event event;

    const std::vector<VkSemaphore> waitSemaphores {imageAvailableSemaphore};
    const std::vector<VkSemaphore> signalSemaphores {renderFinishedSemaphore};

    avocado::vulkan::GraphicsQueue graphicsQueue(logicalDevice.getGraphicsQueue(0));
    graphicsQueue.setSemaphores(waitSemaphores, signalSemaphores);
    graphicsQueue.setPipelineStageFlags({avocado::vulkan::GraphicsQueue::PipelineStageFlag::ColorAttachmentOutput});
    graphicsQueue.setCommandBuffers(cmdBuffers);

    std::vector<avocado::vulkan::Swapchain> swapchains {swapChain};
    presentQueue.setSwapchains(swapchains);
    presentQueue.setWaitSemaphores(signalSemaphores);

    std::vector<uint32_t> imageIndices {swapChain.acquireNextImage(imageAvailableSemaphore)};

    while (true) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;
        }

        logicalDevice.waitForFences(fences, true, std::numeric_limits<uint64_t>::max());
        logicalDevice.resetFences(fences);

        commandBuffer.reset(avocado::vulkan::CommandBuffer::ResetFlags::NoFlags);

        commandBuffer.begin();
        commandBuffer.beginRenderPass(swapChain, renderPassPtr.get(), extent, {0, 0}, imageIndices.front());

        commandBuffer.setViewports(viewPorts);
        commandBuffer.setScissors(scissors);
        commandBuffer.bindPipeline(graphicsPipeline, avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();

        graphicsQueue.submit(fences.front());
        if (graphicsQueue.hasError()) {
            std::cout << "Can't submit graphics queue: " << graphicsQueue.getErrorMessage() << std::endl;
        }

        imageIndices[0] = swapChain.acquireNextImage(imageAvailableSemaphore);
        presentQueue.setImageIndices(imageIndices);
        presentQueue.present();
    }

    logicalDevice.waitIdle();

    // Destroy resources.
    vkDestroySemaphore(logicalDevice.getHandle(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(logicalDevice.getHandle(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(logicalDevice.getHandle(), fences.front(), nullptr);
    vkDestroyCommandPool(logicalDevice.getHandle(), commandPool, nullptr);

    //vkDestroyPipeline(logicalDevice.getHandle(), graphicsPipeline, nullptr);
    //vkDestroyRenderPass(logicalDevice.getHandle(), renderPass, nullptr);

    //vkDestroySurfaceKHR(vkInstance.get(), surface, nullptr);

    return 0;
}
