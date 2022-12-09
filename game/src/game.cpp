#include "gameconfig.hpp"

#include "vertex.hpp"

#include "vulkan/buffer.hpp"
#include "vulkan/clipping.hpp"
#include "vulkan/colorattachment.hpp"
#include "vulkan/commandbuffer.hpp"
#include "vulkan/logicaldevice.hpp"
#include "vulkan/graphicspipeline.hpp"
#include "vulkan/graphicsqueue.hpp"
#include "vulkan/surface.hpp"
#include "vulkan/swapchain.hpp"
#include "vulkan/vertexinputstate.hpp"
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

    physicalDevice.getQueueFamilies(surface); // todo rename this method.
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
        avocado::vulkan::Format::B8G8R8A8SRGB,
        avocado::vulkan::Surface::ColorSpace::SRGBNonlinearKHR);

    if (surface.hasError()) {
        std::cerr << "Can't get surface format: " << surface.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::Swapchain swapChain(logicalDevice);
    swapChain.create(surface, surfaceFormat, extent, imageCount, queueFamilies);
    if (swapChain.hasError()) {
        std::cout << "Can't create swapchain: " << swapChain.getErrorMessage() << std::endl;
        return 1;
    }


    std::vector<avocado::vulkan::Swapchain> swapchains(1);// todo can we do => (1, std::move(swapChain)); ?
    swapchains[0] = std::move(swapChain);
    avocado::vulkan::Swapchain &swapchain = swapchains.front();
    swapchain.getImages();
    if (swapchain.hasError()) {
        std::cout << "Can't get swapchain images: " << swapchain.getErrorMessage() << std::endl;
        return 1;
    }

    swapchain.createImageViews(surfaceFormat);
    if (swapchain.hasError()) {
        std::cout << "Can't get swapchain images: " << swapchain.getErrorMessage() << std::endl;
        return 1;
    }

    if (!std::filesystem::exists(Config::SHADERS_PATH + "/triangle.frag.spv")) {
        std::cout << "File " << Config::SHADERS_PATH << "/triangle.frag.spv doesn't exist." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(Config::SHADERS_PATH + "/triangle.vert.spv")) {
        std::cout << "File " << Config::SHADERS_PATH << "/triangle.vert.spv doesn't exist." << std::endl;
        return 1;
    }
    const std::vector<char> &fragBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.frag.spv");
    const std::vector<char> &vertBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.vert.spv");

    // todo replace to pipeline class.
    const VkPipelineShaderStageCreateInfo &fragShaderModule =
        logicalDevice.addShaderModule(fragBuf, avocado::vulkan::LogicalDevice::ShaderType::Fragment);
    if (logicalDevice.hasError()) {
        std::cout << "Can't add fragment shader module. " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }
    const VkPipelineShaderStageCreateInfo &vertShaderModule =
        logicalDevice.addShaderModule(vertBuf, avocado::vulkan::LogicalDevice::ShaderType::Vertex);
    if (logicalDevice.hasError()) {
        std::cout << "Can't add vertex shader module. " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    const std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCIs = {fragShaderModule, vertShaderModule};

    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder(logicalDevice.getHandle());
    pipelineBuilder.addDynamicStates({
        avocado::vulkan::GraphicsPipelineBuilder::DynamicState::Viewport
        , avocado::vulkan::GraphicsPipelineBuilder::DynamicState::Scissor});

    // Input assembly.
    pipelineBuilder.setPrimitiveTopology(
        avocado::vulkan::GraphicsPipelineBuilder::PrimitiveTopology::TriangleFan);

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

    avocado::vulkan::VertexInputState vertexInState;
    vertexInState.addAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(avocado::Vertex, position));
    vertexInState.addAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(avocado::Vertex, color));
    vertexInState.addBindingDescription(0, sizeof(avocado::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineBuilder.setVertexInputState(vertexInState);

    // Creating vertex buffer.
    const std::vector<avocado::Vertex> triangle {
        /* { pos, color } */
        {{0.f, -0.5f}, {0.f, 0.f, 0.5f}},
        {{0.5f, 0.5f}, {0.f, 0.f, 1.f}},
        {{-0.5f, 0.5f}, {0.f, 0.f, 0.8f}}
    };

    avocado::vulkan::Buffer vertexBuffer(sizeof(decltype(triangle)::value_type) * triangle.size(),
        avocado::vulkan::Buffer::Usage::Vertex,
        avocado::vulkan::Buffer::SharingMode::Exclusive,
        logicalDevice);
    if (vertexBuffer.hasError()) {
        std::cout << "Can't create vertex buffer: " << vertexBuffer.getErrorMessage() << std::endl;
    }

    // Memory requirements.
    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(logicalDevice.getHandle(), vertexBuffer.getHandle(), &memReq);

    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.getHandle(), &memProps);

    // Find needed type of memory by typeFilter & properties.
    uint32_t typeFilter = memReq.memoryTypeBits;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    uint32_t foundType = 0;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            foundType = i;
            break;
        }
    }

    // Allocate memory.
    VkMemoryAllocateInfo memAllocInfo = avocado::vulkan::createStruct<VkMemoryAllocateInfo>();
    memAllocInfo.allocationSize = memReq.size;
    memAllocInfo.memoryTypeIndex = foundType;

    VkDeviceMemory devMem = VK_NULL_HANDLE;
    // todo vkFreeMemory.
    const VkResult allocRes = vkAllocateMemory(logicalDevice.getHandle(), &memAllocInfo, nullptr, &devMem);
    if (allocRes != VK_SUCCESS) {
        std::cout << "Memory alloc error!" << std::endl;
        return 1;
    } else {
        std::cout << "Memory allocated" << std::endl;
    }

    // Bind memory.
    const VkResult bindBufResult = vkBindBufferMemory(logicalDevice.getHandle(), vertexBuffer.getHandle(), devMem, 0);
    if (bindBufResult != VK_SUCCESS) {
        std::cout << "Buffer bind error." << std::endl;
        return 1;
    } else {
        std::cout << "Buffer binded" << std::endl;
    }

    // Map memory.
    void *data = nullptr;
    const VkResult mapRes = vkMapMemory(logicalDevice.getHandle(), devMem, 0, sizeof(decltype(triangle)::value_type) * triangle.size(), 0, &data);
    if (mapRes != VK_SUCCESS) {
        std::cout << "Memory map error" << std::endl;
        return 1;
    } else {
        std::cout << "Memory mapped" << std::endl;
    }

    memcpy(data, triangle.data(), sizeof(decltype(triangle)::value_type) * triangle.size());

    vkUnmapMemory(logicalDevice.getHandle(), devMem);

    auto renderPassPtr = logicalDevice.createRenderPass(surfaceFormat.format);

    const std::vector<VkViewport> viewPorts { avocado::vulkan::Clipping::createViewport(0.f, 0.f, extent) };
    const std::vector<VkRect2D> scissors { avocado::vulkan::Clipping::createScissor(viewPorts.front()) };

    avocado::vulkan::GraphicsPipelineBuilder::PipelineUniquePtr grPip = pipelineBuilder.buildPipeline(pipelineShaderStageCIs, surfaceFormat.format, renderPassPtr.get(), viewPorts, scissors);

    if (grPip == nullptr) {
        std::cout << "INVALID PIPELINE" << std::endl;
        return 1;
    }
    VkPipeline graphicsPipeline = grPip.get();


    swapchain.createFramebuffers(renderPassPtr.get(), extent);

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


    commandBuffer.beginRenderPass(swapchain, renderPassPtr.get(), extent, {0, 0}, 0);


    commandBuffer.bindPipeline(graphicsPipeline, avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
    // Binding vertex buffer.
    VkBuffer vertexBuffers[] {vertexBuffer.getHandle()};
    VkDeviceSize offsets[] {0};

    vkCmdBindVertexBuffers(commandBuffer.getHandle(), 0, 1, vertexBuffers, offsets);
    commandBuffer.setViewports(viewPorts);
    commandBuffer.setScissors(scissors);
    commandBuffer.draw(static_cast<uint32_t>(triangle.size()), 1, 0, 0);
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

    SDL_Event event;

    const std::vector<VkSemaphore> waitSemaphores {imageAvailableSemaphore};
    const std::vector<VkSemaphore> signalSemaphores {renderFinishedSemaphore};

    avocado::vulkan::GraphicsQueue graphicsQueue(logicalDevice.getGraphicsQueue(0));
    logicalDevice.setObjectName(graphicsQueue.getHandle(), "Graphics queue");
    graphicsQueue.setSemaphores(waitSemaphores, signalSemaphores);
    graphicsQueue.setPipelineStageFlags({avocado::vulkan::GraphicsQueue::PipelineStageFlag::ColorAttachmentOutput});

    std::vector<VkCommandBuffer> cmdBufferHandles = avocado::vulkan::getCommandBufferHandles(cmdBuffers);
    graphicsQueue.setCommandBuffers(cmdBufferHandles);


    presentQueue.setSwapchains(swapchains);
    presentQueue.setWaitSemaphores(signalSemaphores);
    const uint32_t imgIndex = swapchain.acquireNextImage(imageAvailableSemaphore);
    if (swapchain.hasError()) {
        std::cout << "Can't get img index " << swapchain.getErrorMessage() << std::endl;
    }

    std::vector<uint32_t> imageIndices {imgIndex};
    presentQueue.setImageIndices(imageIndices);

    while (true) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;
        }

        logicalDevice.waitForFences(fences, true, std::numeric_limits<uint64_t>::max());
        logicalDevice.resetFences(fences);

        commandBuffer.reset(avocado::vulkan::CommandBuffer::ResetFlags::NoFlags);

        commandBuffer.begin();

        commandBuffer.beginRenderPass(swapchain, renderPassPtr.get(), extent, {0, 0}, imageIndices.front());

        commandBuffer.setViewports(viewPorts);
        commandBuffer.setScissors(scissors);
        vkCmdBindVertexBuffers(commandBuffer.getHandle(), 0, 1, vertexBuffers, offsets);
        commandBuffer.bindPipeline(graphicsPipeline, avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
        commandBuffer.draw(static_cast<uint32_t>(triangle.size()), 1, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();

        graphicsQueue.submit(fences.front());
        if (graphicsQueue.hasError()) {
            std::cout << "Can't submit graphics queue: " << graphicsQueue.getErrorMessage() << std::endl;
        }

        presentQueue.present();
        imageIndices[0] = swapchain.acquireNextImage(imageAvailableSemaphore);
    }

    logicalDevice.waitIdle();

    // Destroy resources.
    vkFreeMemory(logicalDevice.getHandle(), devMem, nullptr);
    vkDestroySemaphore(logicalDevice.getHandle(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(logicalDevice.getHandle(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(logicalDevice.getHandle(), fences.front(), nullptr);
    vkDestroyCommandPool(logicalDevice.getHandle(), commandPool, nullptr);

    //vkDestroyPipeline(logicalDevice.getHandle(), graphicsPipeline, nullptr);
    //vkDestroyRenderPass(logicalDevice.getHandle(), renderPass, nullptr);

    //vkDestroySurfaceKHR(vkInstance.get(), surface, nullptr);

    return 0;
}
