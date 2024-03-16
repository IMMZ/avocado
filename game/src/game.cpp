#include "SDL_pixels.h"
#include "SDL_surface.h"
#include "gameconfig.hpp"

#include "vertex.hpp"

#include "vulkan/buffer.hpp"
#include "vulkan/clipping.hpp"
#include "vulkan/commandbuffer.hpp"
#include "vulkan/debugutils.hpp"
#include "vulkan/logicaldevice.hpp"
#include "vulkan/graphicspipeline.hpp"
#include "vulkan/surface.hpp"
#include "vulkan/swapchain.hpp"
#include "vulkan/vkutils.hpp"
#include "vulkantools.hpp" // todo get rid of this header file

#include "vulkan/states/colorblendstate.hpp"
#include "vulkan/states/dynamicstate.hpp"
#include "vulkan/states/inputasmstate.hpp"
#include "vulkan/states/multisamplestate.hpp"
#include "vulkan/states/rasterizationstate.hpp"
#include "vulkan/states/vertexinputstate.hpp"
#include "vulkan/states/viewportstate.hpp"

#include "utils.hpp"

#include "math/functions.hpp"
#include "math/matrix.hpp"
#include "math/vecn.hpp"

#include <core.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_image.h>
#include <SDL_events.h>
#include <SDL_keycode.h>

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <vector>

VkCommandBuffer beginSingleTimeCommands(avocado::vulkan::LogicalDevice &logicalDevice, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice.getHandle(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(avocado::vulkan::LogicalDevice &logicalDevice, VkCommandPool commandPool, avocado::vulkan::Queue &graphicsQueue, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkResult res = vkQueueSubmit(graphicsQueue.getHandle(), 1, &submitInfo, VK_NULL_HANDLE);
    if (res != VK_SUCCESS) {
        std::cout << "QUEUE SUBMIT ERROR" << std::endl;
        return;
    }
    res = vkQueueWaitIdle(graphicsQueue.getHandle());
    if (res != VK_SUCCESS) {
        std::cout << "QUEUE WAIT IDLE ERROR" << std::endl;
        return;
    }

    vkFreeCommandBuffers(logicalDevice.getHandle(), commandPool, 1, &commandBuffer);
}

void transitionImageLayout(avocado::vulkan::LogicalDevice &logicalDevice, VkCommandPool commandPool, avocado::vulkan::Queue &queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice, commandPool);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage, dstStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

    vkCmdPipelineBarrier( commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    endSingleTimeCommands(logicalDevice, commandPool, queue, commandBuffer);
}

void copyBufferToImage(avocado::vulkan::LogicalDevice &logicalDevice, VkCommandPool commandPool, avocado::vulkan::Queue &queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice, commandPool);

    VkBufferImageCopy region{};

    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(logicalDevice, commandPool, queue, commandBuffer);
}

int main(int argc, char ** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "Can't init SDL." << std::endl;
        return 1;
    }

    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> sdlWindow(SDL_CreateWindow(
        Config::GAME_NAME,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        Config::RESOLUTION_WIDTH, Config::RESOLUTION_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN), SDL_DestroyWindow);

    if (sdlWindow == nullptr) {
        std::cerr << "Can't create window." << std::endl;
        return 1;
    }

    // Set window icon.
    const std::string &iconPath = std::filesystem::current_path().string() + "/icon.png";
    {
        std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> iconSurface(IMG_Load(iconPath.c_str()), SDL_FreeSurface);
        if (iconSurface != nullptr)
            SDL_SetWindowIcon(sdlWindow.get(), iconSurface.get());
    }

    avocado::vulkan::Vulkan vlk;
    const std::vector<std::string> instanceLayers {"VK_LAYER_KHRONOS_validation"};
    const bool areLayersSupported = vlk.areLayersSupported(instanceLayers);
    if (!areLayersSupported) {
        std::cerr << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<std::string> instanceExtensions = vlk.getExtensionNamesForSDLSurface(sdlWindow.get());
    if constexpr (avocado::core::isDebugBuild())
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (vlk.hasError()) {
        std::cerr << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    constexpr avocado::vulkan::VulkanInstanceInfo vulkanInfo {
        Config::GAME_NAME,
        0, 1, 0, // App version.
        1, 3 // Vulkan API version.
    };

    vlk.createInstance(instanceExtensions, instanceLayers, vulkanInfo);
    if (vlk.hasError()) {
        std::cerr << vlk.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::PhysicalDevice> physicalDevices = vlk.getPhysicalDevices(); if (vlk.hasError()) {
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

    physicalDevice.initQueueFamilies(surface);
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

    std::vector queueFamilies {graphicsQueueFamily, presentQueueFamily};
    avocado::utils::makeUniqueContainer(queueFamilies);

    avocado::vulkan::LogicalDevice logicalDevice = physicalDevice.createLogicalDevice(queueFamilies, physExtensions, instanceLayers, 1, 1.0f);
    if (physicalDevice.hasError()) {
        std::cerr << "Can't create logical device: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    auto debugUtilsPtr = logicalDevice.createDebugUtils();

    avocado::vulkan::Queue presentQueue = logicalDevice.getPresentQueue(0);

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
    swapChain.create(surface, surfaceFormat, extent, imageCount, queueFamilies); // todo we can forget to call create. Need solution.
    if (swapChain.hasError()) {
        std::cout << "Can't create swapchain: " << swapChain.getErrorMessage() << std::endl;
        return 1;
    }
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

    if (!std::filesystem::exists(Config::SHADERS_PATH + "/triangle.frag.spv")) {
        std::cout << "File " << Config::SHADERS_PATH << "/triangle.frag.spv doesn't exist." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(Config::SHADERS_PATH + "/triangle.vert.spv")) {
        std::cout << "File " << Config::SHADERS_PATH << "/triangle.vert.spv doesn't exist." << std::endl;
        return 1;
    }



    // Building a pipeline.
    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder(logicalDevice);

    const std::vector<char> &fragBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.frag.spv");
    const std::vector<char> &vertBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.vert.spv");
    pipelineBuilder.addVertexShaderModules({vertBuf});
    pipelineBuilder.addFragmentShaderModules({fragBuf});

    avocado::vulkan::DynamicState dynState({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
    pipelineBuilder.setDynamicState(dynState);

    avocado::vulkan::InputAsmState inAsmState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
    pipelineBuilder.setInputAsmState(inAsmState);

    avocado::vulkan::RasterizationState rastState;
    rastState.setDepthBiasEnabled(VK_FALSE);
    rastState.setPolygonMode(VK_POLYGON_MODE_FILL);
    rastState.setCullMode(VK_CULL_MODE_BACK_BIT);
    rastState.setFrontFace(VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.setRasterizationState(rastState);

    avocado::vulkan::MultisampleState multisampleState;
    multisampleState.setRasterizationSamples(VK_SAMPLE_COUNT_1_BIT);
    pipelineBuilder.setMultisampleState(multisampleState);

    avocado::vulkan::ColorBlendState colorBlendState;
    colorBlendState.setLogicOp(VK_LOGIC_OP_COPY);
    colorBlendState.addAttachment({
        VK_TRUE // Blend enabled.
        , VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD // Color blend factor.
        , VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD // Alpha blend factor.
        , VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT // Color write mask.
        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});
    pipelineBuilder.setColorBlendState(colorBlendState);

    avocado::vulkan::VertexInputState vertexInState;
    vertexInState.addAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(avocado::Vertex, position));
    vertexInState.addAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(avocado::Vertex, color));
    vertexInState.addAttributeDescription(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(avocado::Vertex, textureCoordinate));
    vertexInState.addBindingDescription(0, sizeof(avocado::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineBuilder.setVertexInputState(vertexInState);

    constexpr std::array<avocado::Vertex, 4> quad = {{
        /* { pos, color, textureCoordinates } */
        {avocado::math::vec2f(-.5f, -.5f), avocado::math::vec3f(1.f, 0.f, 0.f), avocado::math::vec2f(1.f, 0.f)},
        {avocado::math::vec2f( .5f, -.5f), avocado::math::vec3f(0.f, 1.f, 0.f), avocado::math::vec2f(0.f, 0.f)},
        {avocado::math::vec2f( .5f,  .5f), avocado::math::vec3f(0.f, 0.f, 1.f), avocado::math::vec2f(0.f, 1.f)},
        {avocado::math::vec2f(-.5f,  .5f), avocado::math::vec3f(1.f, 1.f, 1.f), avocado::math::vec2f(1.f, 1.f)}
    }};
    constexpr VkDeviceSize verticesSizeBytes = sizeof(decltype(quad)::value_type) * quad.size();
    constexpr std::array<uint16_t, 6> indices {0, 1, 2, 2, 3, 0};
    constexpr size_t indicesSizeBytes = indices.size() * sizeof(decltype(indices)::value_type);

    avocado::vulkan::Buffer vertexBuffer(verticesSizeBytes,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
        VK_SHARING_MODE_EXCLUSIVE,
        logicalDevice, physicalDevice);

    if (vertexBuffer.hasError()) {
        std::cout << "Can't create vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (vertexBuffer.hasError()) {
        std::cout << "Can't allocate memory on vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.fill(quad.data(), quad.size() * sizeof(avocado::Vertex));

    vertexBuffer.bindMemory();

    avocado::vulkan::Buffer indexBuffer(indicesSizeBytes,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
        VK_SHARING_MODE_EXCLUSIVE, logicalDevice, physicalDevice);
    if (indexBuffer.hasError()) {
        std::cout << "Can't create index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }
    indexBuffer.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (indexBuffer.hasError()) {
        std::cout << "Can't allocate memory on index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }



    indexBuffer.fill(indices.data(), indices.size() * sizeof(uint16_t));

    indexBuffer.bindMemory();
    if (indexBuffer.hasError()) {
        std::cout << "Can't bind memory of index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    struct UniformBufferObject {
        alignas(16) avocado::math::Mat4x4 model;
        alignas(16) avocado::math::Mat4x4 proj;
        alignas(16) avocado::math::Mat4x4 view;
    };

    avocado::vulkan::Buffer uniformBuffer(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, logicalDevice, physicalDevice);
    uniformBuffer.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    avocado::vulkan::Buffer uniformBuffer1(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, logicalDevice, physicalDevice);
    uniformBuffer1.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBuffer1.bindMemory();

    UniformBufferObject ubo{};
    //ubo.view = avocado::math::lookAt(avocado::math::vec3f(2.f, 2.f, 2.f), avocado::math::vec3f(0.f, 0.f, 0.f), avocado::math::vec3f(0.f, 0.f, 1.f));
    ubo.view = avocado::math::lookAt(avocado::math::vec3f(0.f, 0.f, 2.f), avocado::math::vec3f(0.f, 0.f, 0.f), avocado::math::vec3f(0.f, 1.f, 0.f));
    ubo.proj = avocado::math::perspectiveProjection(45.f, static_cast<float>(Config::RESOLUTION_WIDTH) / static_cast<float>(Config::RESOLUTION_HEIGHT), 0.1f, 10.f);
    uniformBuffer.fill(&ubo, sizeof(ubo));
    uniformBuffer.bindMemory();

    auto renderPassPtr = logicalDevice.createRenderPass(surfaceFormat.format);
    const std::vector<VkViewport> viewPorts { avocado::vulkan::Clipping::createViewport(0.f, 0.f, extent) };
    const std::vector<VkRect2D> scissors { avocado::vulkan::Clipping::createScissor(viewPorts.front()) };
    avocado::vulkan::ViewportState viewportState(viewPorts, scissors);
    pipelineBuilder.setViewportState(viewportState);


    // Create descriptor set layout.
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        std::cout << "failed to create descriptor set layout!" << std::endl;
        return 1;
    }

    avocado::vulkan::ObjectPtr<VkDescriptorSetLayout> descriptorSetLayoutPtr = logicalDevice.createObjectPointer(descriptorSetLayout);

    std::vector<VkDescriptorSet> descriptorSets;
    avocado::vulkan::ObjectPtr<VkDescriptorPool> descriptorPool = logicalDevice.createObjectPointer(logicalDevice.createDescriptorPool());

    std::array<avocado::vulkan::Buffer*, 2> uniformBuffers = {&uniformBuffer, &uniformBuffer1};

    // Create descriptor sets.
    std::vector<VkDescriptorSetLayout> layouts(2, descriptorSetLayoutPtr.get());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool.get();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(2);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(2);
    const VkResult ads = vkAllocateDescriptorSets(logicalDevice.getHandle(), &allocInfo, descriptorSets.data());
    if (ads != VK_SUCCESS) {
        throw std::runtime_error("Can't allocate descriptor sets"s + std::to_string(static_cast<int>(ads)));
    }

    pipelineBuilder.setDSLayouts(layouts);

    avocado::vulkan::GraphicsPipelineBuilder::PipelineUniquePtr graphicsPipeline = pipelineBuilder.buildPipeline(renderPassPtr.get());

    if (graphicsPipeline == nullptr) {
        std::cout << "Error: invalid pipeline" << std::endl;
        return 1;
    }

    avocado::vulkan::ObjectPtr<VkCommandPool> commandPool = logicalDevice.createObjectPointer(logicalDevice.createCommandPool(avocado::vulkan::LogicalDevice::CommandPoolCreationFlags::Reset, graphicsQueueFamily));
    if (logicalDevice.hasError()) {
        std::cout << "Can't create command pool: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::CommandBuffer> cmdBuffers = logicalDevice.allocateCommandBuffers(1, commandPool.get(), avocado::vulkan::LogicalDevice::CommandBufferLevel::Primary);
    if (logicalDevice.hasError()) {
        std::cout << "Can't allocate command buffers (" << logicalDevice.getErrorMessage() << ")." << std::endl;
        return 1;
    }

    VkBuffer vertexBuffers[] {vertexBuffer.getHandle()};
    VkDeviceSize offsets[] {0};
    avocado::vulkan::CommandBuffer commandBuffer = cmdBuffers.front();

    // Synchronization objects.
    avocado::vulkan::ObjectPtr<VkSemaphore> imageAvailableSemaphore = logicalDevice.createObjectPointer(logicalDevice.createSemaphore());
    if (logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::ObjectPtr<VkSemaphore> renderFinishedSemaphore = logicalDevice.createObjectPointer(logicalDevice.createSemaphore());
    if (logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::ObjectPtr<VkFence> fence = logicalDevice.createObjectPointer(logicalDevice.createFence());
    std::vector<VkFence> fences {fence.get()};
    if (logicalDevice.hasError()) {
        std::cout << "Can't create fence: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    const std::vector<VkSemaphore> waitSemaphores {imageAvailableSemaphore.get()};
    const std::vector<VkSemaphore> signalSemaphores {renderFinishedSemaphore.get()};

    avocado::vulkan::Queue graphicsQueue(logicalDevice.getGraphicsQueue(0));
    debugUtilsPtr->setObjectName(graphicsQueue.getHandle(), "Graphics queue");

    std::vector cmdBufferHandles = avocado::vulkan::getCommandBufferHandles(cmdBuffers);

    const uint32_t imgIndex = swapChain.acquireNextImage(imageAvailableSemaphore.get());
    if (swapChain.hasError()) {
        std::cout << "Can't get img index " << swapChain.getErrorMessage() << std::endl;
    }

    std::vector imageIndices {imgIndex};

    std::vector<VkPipelineStageFlags> flags = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    auto submitInfo = graphicsQueue.createSubmitInfo(waitSemaphores, signalSemaphores, cmdBufferHandles, flags);
    SDL_Event event;
    std::vector<VkSwapchainKHR> swapChainHandles{swapChain.getHandle()};

    // Load image
    constexpr const char * const imgPath = "./game/bin/tusya.jpg";
    int imgW = 0; int imgH = 0;
    VkDeviceSize imgSize = VkDeviceSize(0);
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> imgSurface(IMG_Load(imgPath), SDL_FreeSurface);
    if (imgSurface == nullptr) {
        std::cout << "Error loading image (" << imgPath << ")" << std::endl;
        return 1;
    }

    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> convertedSurface(SDL_ConvertSurfaceFormat(imgSurface.get(), SDL_PIXELFORMAT_RGBA32, 0), SDL_FreeSurface);
    if (convertedSurface == nullptr) {
        std::cout << "Error converting surface to RGBA32" << std::endl;
        return 1;
    }

    imgW = convertedSurface->w;
    imgH = convertedSurface->h;
    imgSize = imgW * imgH * convertedSurface->format->BytesPerPixel;

    avocado::vulkan::Buffer imgTransferBuffer(imgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, logicalDevice, physicalDevice);
    imgTransferBuffer.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    imgTransferBuffer.fill(convertedSurface->pixels, imgSize);
    imgTransferBuffer.bindMemory();

    // Create image.
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    auto vkImageCI = avocado::vulkan::createStruct<VkImageCreateInfo>();
    vkImageCI.imageType = VK_IMAGE_TYPE_2D;
    vkImageCI.extent.width = imgW;
    vkImageCI.extent.height = imgH;
    vkImageCI.extent.depth = 1;
    vkImageCI.mipLevels = 1;
    vkImageCI.arrayLayers = 1;
    vkImageCI.format = VK_FORMAT_R8G8B8A8_SRGB;
    vkImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    VkResult v1 = vkCreateImage(logicalDevice.getHandle(), &vkImageCI, nullptr, &textureImage);
    if (v1 != VK_SUCCESS) {
        std::cout << "vkCreateImage error" << std::endl;
        return 1;
    }

    // Setup image memory. Allocate and bind it.
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logicalDevice.getHandle(), textureImage, &memRequirements);

    auto imageMemAI = avocado::vulkan::createStruct<VkMemoryAllocateInfo>();
    imageMemAI.allocationSize = memRequirements.size;

    // todo Do the func for buffer & image for found type index.
    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.getHandle(), &memProps);

    const uint32_t typeFilter = memRequirements.memoryTypeBits;

    uint32_t foundType = 0;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            foundType = i;
            break;
        }
    }

    imageMemAI.memoryTypeIndex = foundType;
    const VkResult v2 = vkAllocateMemory(logicalDevice.getHandle(), &imageMemAI, nullptr, &textureImageMemory);
    if (v2 != VK_SUCCESS) {
        std::cout << "vkAllocateMemory error" << std::endl;
        return 1;
    }

    const VkResult v3 = vkBindImageMemory(logicalDevice.getHandle(), textureImage, textureImageMemory, 0);
    if (v3 != VK_SUCCESS) {
        std::cout << "vkBindImageMemory error" << std::endl;
        return 1;
    }

    avocado::vulkan::ObjectPtr<VkImageView> textureImageView = logicalDevice.createObjectPointer(swapChain.createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB));
    swapChain.createFramebuffers(renderPassPtr.get(), extent);

    transitionImageLayout(logicalDevice, commandPool.get(), graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(logicalDevice, commandPool.get(), graphicsQueue, imgTransferBuffer.getHandle(), textureImage, static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH));
    transitionImageLayout(logicalDevice, commandPool.get(), graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.anisotropyEnable = VK_FALSE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice.getHandle(), &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler textureSampler;
    vkCreateSampler(logicalDevice.getHandle(), &samplerInfo, nullptr, &textureSampler);
    avocado::vulkan::ObjectPtr<VkSampler> textureSamplerPtr = logicalDevice.createObjectPointer(textureSampler);

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    for (size_t i = 0; i < 2; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i]->getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;


        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView.get();
        imageInfo.sampler = textureSamplerPtr.get();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;


        vkUpdateDescriptorSets(logicalDevice.getHandle(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Main loop.
    while (true) {
        if (SDL_PollEvent(&event)) {
            if ((event.type == SDL_QUIT) || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                break;
        }

        logicalDevice.waitForFences(fences, true, std::numeric_limits<uint64_t>::max());
        logicalDevice.resetFences(fences);

        // Update uniform buffer.
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        ubo.model = avocado::math::Mat4x4::createIdentityMatrix() * avocado::math::createRotationMatrix(time * 90.f, avocado::math::vec3f(0.0f, 0.0f, 1.0f));
        uniformBuffer.fill(&ubo, sizeof(ubo));

        commandBuffer.reset(avocado::vulkan::CommandBuffer::ResetFlags::NoFlags);
        commandBuffer.begin();
        commandBuffer.beginRenderPass(swapChain, renderPassPtr.get(), extent, {0, 0}, imageIndices.front());

        commandBuffer.setViewports(viewPorts);
        commandBuffer.setScissors(scissors);
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(indexBuffer.getHandle(), 0, avocado::vulkan::toIndexType<decltype(indices)::value_type>());
        commandBuffer.bindPipeline(graphicsPipeline.get(), avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
        commandBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBuilder.getPipelineLayout(), 0,descriptorSets.size(), descriptorSets.data(), 0, nullptr);
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();

        graphicsQueue.submit(submitInfo, fences.front());
        if (graphicsQueue.hasError()) {
            std::cout << "Can't submit graphics queue: " << graphicsQueue.getErrorMessage() << std::endl;
        }

        presentQueue.present(signalSemaphores, imageIndices, swapChainHandles);
        imageIndices[0] = swapChain.acquireNextImage(imageAvailableSemaphore.get());
    } // main loop.

    logicalDevice.waitIdle();

    // Destroy resources.
    vkFreeMemory(logicalDevice.getHandle(), textureImageMemory, nullptr);
    vkDestroyImage(logicalDevice.getHandle(), textureImage, nullptr);
    pipelineBuilder.destroyPipeline();

    return 0;
}
