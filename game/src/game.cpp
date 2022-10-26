#include "gameconfig.hpp"

#include "vulkan/colorattachment.hpp"
#include "vulkan/commandbuffer.hpp"
#include "vulkan/logicaldevice.hpp"
#include "vulkan/graphicspipeline.hpp"
#include "vulkan/surface.hpp"
#include "vulkan/swapchain.hpp"
#include "vulkantools.hpp"

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

    
    avocado::vulkan::VulkanInstanceInfo vii;
    vii.appName = Config::GAME_NAME;
    vii.appMajorVersion = 0; vii.appMinorVersion = 1; vii.appPatchVersion = 0;
    vii.apiMajorVersion = 1; vii.apiMinorVersion = 2;
    vlk.createInstance(instanceExtensions, instanceLayers, vii);
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
    
    // Get graphics queue family's index. 
    avocado::vulkan::PhysicalDevice &physicalDevice = physicalDevices.front();

    avocado::vulkan::Surface surface = vlk.createSurface(sdlWindow.get(), physicalDevice);
    if (vlk.hasError()) {
        std::cerr << "Can't create surface: " << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    const auto &queueFamilies = physicalDevice.getQueueFamilies();
    const uint32_t graphicsQueueFamilyI = physicalDevice.getGraphicsQueueFamilyIndex(queueFamilies);
    const uint32_t presentQueueFamilyI = surface.getPresentQueueFamilyIndex(queueFamilies);
    if (surface.hasError()) {
        std::cout << "Can't get present queue family index: " << surface.getErrorMessage() << std::endl;
        return 1;
    }

    // Don't forget - we must have a vector with unique queue family indices to create a logical device.
    const std::set<uint32_t> _uniqueQueueFamilyIndices = {graphicsQueueFamilyI, presentQueueFamilyI};
    const std::vector<decltype(_uniqueQueueFamilyIndices)::value_type> uniqueQueueFamilyIndices(
        _uniqueQueueFamilyIndices.cbegin(), _uniqueQueueFamilyIndices.cend());

    std::vector<std::string> physExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    physicalDevice.areExtensionsSupported(physExtensions);
    if (physicalDevice.hasError()) {
        std::cerr << "Extensions error: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::LogicalDevice logicalDevice = physicalDevice.createLogicalDevice(uniqueQueueFamilyIndices, physExtensions, instanceLayers, 1, 1.0f);
    if (physicalDevice.hasError()) {
        std::cerr << "Can't create logical device: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    // Retrieve graphics queue handle.
    VkQueue graphicsQueue = logicalDevice.getQueue(graphicsQueueFamilyI, 0);
    VkQueue presentQueue = logicalDevice.getQueue(presentQueueFamilyI, 0);

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
    std::vector<uint32_t> queueIndices {graphicsQueueFamilyI, presentQueueFamilyI};
    avocado::utils::makeUniqueContainer(queueIndices);
    swapChain.create(surface, surfaceFormat, extent, imageCount, queueIndices);

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
    const VkPipelineShaderStageCreateInfo &fragShaderModule = 
        logicalDevice.addShaderModule(fragBuf, avocado::vulkan::LogicalDevice::ShaderType::Fragment);
    const VkPipelineShaderStageCreateInfo &vertShaderModule = logicalDevice.addShaderModule(vertBuf, avocado::vulkan::LogicalDevice::ShaderType::Vertex);
    const std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCIs = {fragShaderModule, vertShaderModule};

    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder(logicalDevice.getHandle());
    pipelineBuilder.addDynamicStates({
        avocado::vulkan::GraphicsPipelineBuilder::DynamicState::Viewport
        , avocado::vulkan::GraphicsPipelineBuilder::DynamicState::Scissor});

    // Input assembly.
    pipelineBuilder.setPrimitiveTopology(
        avocado::vulkan::GraphicsPipelineBuilder::PrimitiveTopology::TriangleList);

    // Viewport.
    pipelineBuilder.setViewPortSize(extent.width, extent.height);
    pipelineBuilder.setViewPortMaxDepth(1.0f);

    // Rasterizer.
    pipelineBuilder.setPolygonMode(
        avocado::vulkan::GraphicsPipelineBuilder::PolygonMode::Fill);
    pipelineBuilder.setCullMode(
        avocado::vulkan::GraphicsPipelineBuilder::CullMode::BackBit);
    pipelineBuilder.setFrontFace(
        avocado::vulkan::GraphicsPipelineBuilder::FrontFace::Clockwise);

    // Multisampling.
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
    avocado::vulkan::GraphicsPipelineBuilder::PipelineUniquePtr grPip = pipelineBuilder.buildPipeline(pipelineShaderStageCIs, surfaceFormat.format, renderPassPtr.get());

    if (grPip == nullptr) {
        std::cout << "INVALID PIPELINE" << std::endl;
        return 1;
    }
    VkPipeline graphicsPipeline = grPip.get();

    swapChain.createFramebuffers(renderPassPtr.get(), extent);

    VkCommandPool commandPool = logicalDevice.createCommandPool(avocado::vulkan::LogicalDevice::CommandPoolCreationFlags::Reset, graphicsQueueFamilyI);
    if (logicalDevice.hasError()) {
        std::cout << "Can't create command pool: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    const std::vector<avocado::vulkan::CommandBuffer> &cmdBuffers = logicalDevice.allocateCommandBuffers(1, commandPool, avocado::vulkan::LogicalDevice::CommandBufferLevel::Primary);
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
    
    commandBuffer.beginRenderPass(swapChain, renderPassPtr.get(), extent, {0, 0});

    commandBuffer.bindPipeline(graphicsPipeline, avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
    commandBuffer.setViewport(pipelineBuilder.getViewport());
    commandBuffer.setScissor(pipelineBuilder.getScissor());
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
    while (true) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;
        }
        
        // Draw procedure.
        logicalDevice.waitForFences(fences, true, std::numeric_limits<uint64_t>::max());
        logicalDevice.resetFences(fences);

        const uint32_t imageIndex = swapChain.acquireNextImage(imageAvailableSemaphore);
        commandBuffer.reset(avocado::vulkan::CommandBuffer::ResetFlags::NoFlags);
        commandBuffer.begin();

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPassPtr.get();
        renderPassInfo.framebuffer = swapChain.getFramebuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer.getHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        commandBuffer.setViewport(pipelineBuilder.getViewport());
        commandBuffer.setScissor(pipelineBuilder.getScissor());
        
        commandBuffer.bindPipeline(graphicsPipeline, avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
        commandBuffer.end();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        auto cmdBufHandle = commandBuffer.getHandle();
        submitInfo.pCommandBuffers = &cmdBufHandle;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fences.front()) != VK_SUCCESS) {
            std::cout << "Can't submit draw command buffer" << std::endl;
            return 1;
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain.getHandle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(presentQueue, &presentInfo);
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
