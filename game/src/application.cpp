#include "application.hpp"

#include "gameconfig.hpp"
#include "vertex.hpp"
#include "utils.hpp"
#include "vulkan/graphicspipeline.hpp"

#include <math/functions.hpp>
#include <math/matrix.hpp>
#include <math/vecn.hpp>

#include <vulkan/buffer.hpp>
#include <vulkan/clipping.hpp>
#include <vulkan/commandbuffer.hpp>
#include <vulkan/debugutils.hpp>
#include <vulkan/image.hpp>
#include <vulkan/logicaldevice.hpp>
#include <vulkan/pointertypes.hpp>
#include <vulkan/surface.hpp>
#include <vulkan/swapchain.hpp>
#include <vulkan/vkutils.hpp>

#include <core.hpp>

#include <SDL_vulkan.h>
#include <SDL_image.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_surface.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/structuretypes.hpp>

#include <filesystem>
#include <iostream>
#include <memory>

VkCommandBuffer beginSingleTimeCommands(avocado::vulkan::LogicalDevice &logicalDevice, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    FILL_S_TYPE(allocInfo);

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

void transitionImageLayout(avocado::vulkan::LogicalDevice &logicalDevice, VkCommandPool commandPool, avocado::vulkan::Queue &queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageAspectFlags aspectFlags) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(logicalDevice, commandPool);
    VkImageMemoryBarrier barrier{}; FILL_S_TYPE(barrier);
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectFlags;
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
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!"); // todo No exceptions!
        }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    endSingleTimeCommands(logicalDevice, commandPool, queue, commandBuffer);
}

void copyBufferToImage(avocado::vulkan::LogicalDevice &logicalDevice, VkCommandPool commandPool, avocado::vulkan::Queue &queue, avocado::vulkan::Buffer &buffer,
    avocado::vulkan::Image &image, uint32_t width, uint32_t height) {
    const std::vector commandBuffers = logicalDevice.allocateCommandBuffers(1, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    avocado::vulkan::CommandBuffer commandBuffer = commandBuffers.front();

    commandBuffer.begin();
    commandBuffer.copyBufferToImage(buffer, image, width, height);
    commandBuffer.end();

    VkCommandBuffer bufferHandle = commandBuffer.getHandle();
    VkSubmitInfo submitInfo{}; FILL_S_TYPE(submitInfo);
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &bufferHandle;
    queue.submit(submitInfo);
    if (queue.hasError()) {
        std::cout << "Queue submit error (" << queue.getErrorMessage() << ')' << std::endl;
        return;
    }

    queue.waitIdle();
    if (queue.hasError()) {
        std::cout << "Queue wait idle error (" << queue.getErrorMessage() << ')' << std::endl;
        return;
    }
}

void Application::createInstance(SDL_Window &window, const std::vector<std::string> &instanceLayers) {
    const bool areLayersSupported = _vulkan.areLayersSupported(instanceLayers);
    if (!areLayersSupported) {
        std::cerr << _vulkan.getErrorMessage() << std::endl;
        return;
    }

    std::vector<std::string> instanceExtensions = _vulkan.getExtensionNamesForSDLSurface(&window);
    if constexpr (avocado::core::isDebugBuild())
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (_vulkan.hasError()) {
        std::cerr << _vulkan.getErrorMessage() << std::endl;
        return;
    }

    constexpr avocado::vulkan::VulkanInstanceInfo vulkanInfo {
        Config::GAME_NAME,
        0, 1, 0, // App version.
        1, 3 // Vulkan API version.
    };

    _vulkan.createInstance(instanceExtensions, instanceLayers, vulkanInfo);
    if (_vulkan.hasError()) {
        std::cerr << _vulkan.getErrorMessage() << std::endl;
        return;
    }
}

void Application::createPhysicalDevice() {
    std::vector<avocado::vulkan::PhysicalDevice> physicalDevices = _vulkan.getPhysicalDevices();
    if (_vulkan.hasError()) {
        std::cout << "Can't get physical devices: " << _vulkan.getErrorMessage() << std::endl;
        return;
    }

    if (physicalDevices.empty()) {
        std::cout << "No physical devices found." << std::endl;
        return;
    }

    _physicalDevice = std::move(physicalDevices.front()); // Extract and write some GPU picking algo.
}

avocado::vulkan::Swapchain Application::createSwapchain(avocado::vulkan::Surface &surface, const VkSurfaceFormatKHR surfaceFormat, const VkExtent2D extent,
    const std::vector<avocado::vulkan::QueueFamily> &queueFamilies) {
    avocado::vulkan::Swapchain swapChain(_logicalDevice);
    uint32_t imageCount = surface.getMinImageCount() + 1;
    if (surface.getMaxImageCount() > 0 && imageCount > surface.getMaxImageCount())
        imageCount = surface.getMaxImageCount();

    if (surface.hasError()) {
        std::cerr << "Can't get surface format: " << surface.getErrorMessage() << std::endl;
        return swapChain;
    }

    swapChain.create(surface, surfaceFormat, extent, imageCount, queueFamilies); // todo we can forget to call create. Need solution.
    if (swapChain.hasError()) {
        std::cout << "Can't create swapchain: " << swapChain.getErrorMessage() << std::endl;
        return swapChain;
    }
    swapChain.getImages();
    if (swapChain.hasError()) {
        std::cout << "Can't get swapchain images: " << swapChain.getErrorMessage() << std::endl;
        return swapChain;
    }

    swapChain.createImageViews(surfaceFormat);

    if (swapChain.hasError())
        std::cout << "Can't get swapchain images: " << swapChain.getErrorMessage() << std::endl;

    return swapChain;
}

std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> Application::createWindow() {
    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> sdlWindow(SDL_CreateWindow(
        Config::GAME_NAME,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        Config::RESOLUTION_WIDTH, Config::RESOLUTION_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN), SDL_DestroyWindow);

    // Set window icon.
    if (sdlWindow != nullptr) {
        const std::string &iconPath = std::filesystem::current_path().string() + "/icon.png";
        {
            std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> iconSurface(IMG_Load(iconPath.c_str()), SDL_FreeSurface);
            if (iconSurface != nullptr)
                SDL_SetWindowIcon(sdlWindow.get(), iconSurface.get());
        }
    }

    return sdlWindow;
}

// todo How to find out, that this function returns error?
avocado::vulkan::GraphicsPipelineBuilder Application::preparePipeline(const VkExtent2D extent,
    std::vector<VkDescriptorSetLayout> &layouts, const std::vector<VkViewport> &viewPorts,
    const std::vector<VkRect2D> &scissors) {
    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder(_logicalDevice);
    pipelineBuilder.loadShaders(Config::SHADERS_PATH);
    pipelineBuilder.setDynamicStates({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
    pipelineBuilder.createDynamicState();

    VkPipelineInputAssemblyStateCreateInfo &inAsmState = pipelineBuilder.createInputAssmeblyState();
    inAsmState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo &rastState = pipelineBuilder.createRasterizationState();
    rastState.depthClampEnable = VK_FALSE;
    rastState.rasterizerDiscardEnable = VK_FALSE;
    rastState.depthBiasEnable = VK_FALSE;
    rastState.polygonMode = VK_POLYGON_MODE_FILL;
    rastState.cullMode = VK_CULL_MODE_BACK_BIT;
    rastState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rastState.lineWidth = 1.f;

    VkPipelineDepthStencilStateCreateInfo &depthStencilState = pipelineBuilder.createDepthStencilState();
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.minDepthBounds = 0.f;
    depthStencilState.maxDepthBounds = 1.f;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo &multisampleState = pipelineBuilder.createMultisampleState();
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.minSampleShading = 0.f;

    VkPipelineColorBlendStateCreateInfo &colorBlendState = pipelineBuilder.createColorBlendState();
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    pipelineBuilder.addColorBlendAttachment(
            { VK_TRUE // Blend enabled.
            , VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD // Color blend factor.
            , VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD // Alpha blend factor.
            , VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT // Color write mask.
            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});

    VkPipelineVertexInputStateCreateInfo &vertexInState = pipelineBuilder.createVertexInputState();
    pipelineBuilder.addAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(avocado::Vertex, position));
    pipelineBuilder.addAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(avocado::Vertex, color));
    pipelineBuilder.addAttributeDescription(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(avocado::Vertex, textureCoordinate));
    pipelineBuilder.addBindingDescription(0, sizeof(avocado::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

    VkPipelineViewportStateCreateInfo &viewportState = pipelineBuilder.createViewportState();
    pipelineBuilder.setViewPorts(viewPorts);
    pipelineBuilder.setScissors(scissors);

    pipelineBuilder.setDescriptorSetLayouts(layouts);

    return pipelineBuilder;
}

void Application::updateDescriptorSets(std::vector<avocado::vulkan::Buffer*> &uniformBuffers, const VkDeviceSize range, std::vector<VkDescriptorSet> &descriptorSets,
    avocado::vulkan::ImageViewPtr &textureImageView, avocado::vulkan::SamplerPtr &textureSampler) {
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    for (size_t i = 0; i < 2; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i]->getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = range;

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
        imageInfo.sampler = textureSampler.get();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(_logicalDevice.getHandle(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

int Application::run() {
    const bool isInitOk = init();
    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> sdlWindow = createWindow();
    if (sdlWindow == nullptr) {
        std::cerr << "Can't create window." << std::endl;
        return 1;
    }

    const std::vector<std::string> instanceLayers {"VK_LAYER_KHRONOS_validation"};
    createInstance(*sdlWindow, instanceLayers);
    createPhysicalDevice();

    const std::vector<std::string> physExtensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const bool areExtensionsSupported = _physicalDevice.areExtensionsSupported(physExtensions);
    if (_physicalDevice.hasError()) {
        std::cerr << "Extensions error: " << _physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    if (!areExtensionsSupported) {
        std::cerr << "Required extensions are not supported." << std::endl;
        return 1;
    }

    avocado::vulkan::Surface surface = _vulkan.createSurface(sdlWindow.get(), _physicalDevice);
    if (_vulkan.hasError()) {
        std::cerr << "Can't create surface: " << _vulkan.getErrorMessage() << std::endl;
        return 1;
    }

    _physicalDevice.initQueueFamilies(surface);
    if (_physicalDevice.hasError()) {
        std::cout << "Can't get queue families: " << _physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }
    const avocado::vulkan::QueueFamily graphicsQueueFamily = _physicalDevice.getGraphicsQueueFamily();
    const avocado::vulkan::QueueFamily presentQueueFamily = _physicalDevice.getPresentQueueFamily();

    if (surface.hasError()) {
        std::cout << "Can't get present queue family index: " << surface.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector queueFamilies {graphicsQueueFamily, presentQueueFamily};
    avocado::utils::makeUniqueContainer(queueFamilies);

    _logicalDevice = _physicalDevice.createLogicalDevice(queueFamilies, physExtensions, instanceLayers, 1, 1.0f);
    if (_physicalDevice.hasError()) {
        std::cerr << "Can't create logical device: " << _physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    auto debugUtilsPtr = _logicalDevice.createDebugUtils();

    avocado::vulkan::Queue presentQueue = _logicalDevice.getPresentQueue(0);

    VkExtent2D extent = surface.getCapabilities(sdlWindow.get());

    // Clamp width and height to fit into capabilities.
    extent.width = std::clamp(extent.width, surface.getMinExtentW(), surface.getMaxExtentW());
    extent.height = std::clamp(extent.height, surface.getMinExtentH(), surface.getMaxExtentH());

    // Creating swapchain.
    const VkSurfaceFormatKHR surfaceFormat = surface.findFormat(VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    avocado::vulkan::Swapchain swapChain = createSwapchain(surface, surfaceFormat, extent, queueFamilies);

    constexpr std::array<avocado::Vertex, 8> myModel = {{
        // { pos, color, textureCoordinates }
        {avocado::math::vec3f(-.5f, -.5f, 0.f), avocado::math::vec3f(1.f, 0.f, 0.f), avocado::math::vec2f(1.f, 0.f)},
        {avocado::math::vec3f( .5f, -.5f, 0.f), avocado::math::vec3f(0.f, 1.f, 0.f), avocado::math::vec2f(0.f, 0.f)},
        {avocado::math::vec3f( .5f,  .5f, 0.f), avocado::math::vec3f(0.f, 0.f, 1.f), avocado::math::vec2f(0.f, 1.f)},
        {avocado::math::vec3f(-.5f,  .5f, 0.f), avocado::math::vec3f(1.f, 1.f, 1.f), avocado::math::vec2f(1.f, 1.f)},

        {avocado::math::vec3f(-.5f, -.5f, -.5f), avocado::math::vec3f(1.f, 0.f, 0.f), avocado::math::vec2f(1.f, 0.f)},
        {avocado::math::vec3f( .5f, -.5f, -.5f), avocado::math::vec3f(0.f, 1.f, 0.f), avocado::math::vec2f(0.f, 0.f)},
        {avocado::math::vec3f( .5f,  .5f, -.5f), avocado::math::vec3f(0.f, 0.f, 1.f), avocado::math::vec2f(0.f, 1.f)},
        {avocado::math::vec3f(-.5f,  .5f, -.5f), avocado::math::vec3f(1.f, 1.f, 1.f), avocado::math::vec2f(1.f, 1.f)}
    }};

    constexpr VkDeviceSize verticesSizeBytes = sizeof(decltype(myModel)::value_type) * myModel.size();
    constexpr std::array<uint16_t, 12> indices {
        0, 1, 2, 2, 3, 0, // Draw 1st plane.
        4, 5, 6, 6, 7, 4 // Draw 2nd one.
    };
    constexpr size_t indicesSizeBytes = indices.size() * sizeof(decltype(indices)::value_type);

    avocado::vulkan::Buffer vertexBuffer(verticesSizeBytes,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
        VK_SHARING_MODE_EXCLUSIVE,
        _logicalDevice);

    if (vertexBuffer.hasError()) {
        std::cout << "Can't create vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (vertexBuffer.hasError()) {
        std::cout << "Can't allocate memory on vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.fill(myModel.data());
    vertexBuffer.bindMemory();

    avocado::vulkan::Buffer indexBuffer(indicesSizeBytes,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
        VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    if (indexBuffer.hasError()) {
        std::cout << "Can't create index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }
    indexBuffer.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (indexBuffer.hasError()) {
        std::cout << "Can't allocate memory on index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    indexBuffer.fill(indices.data());
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

    avocado::vulkan::Buffer uniformBuffer(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    uniformBuffer.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBuffer.bindMemory();

    avocado::vulkan::Buffer uniformBufferForFrame2(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    uniformBufferForFrame2.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBufferForFrame2.bindMemory();

    UniformBufferObject ubo{};
    ubo.view = avocado::math::lookAt(avocado::math::vec3f(2.f, 2.f, 2.f), avocado::math::vec3f(0.f, 0.f, 0.f), avocado::math::vec3f(0.f, 0.f, 1.f));
    ubo.proj = avocado::math::perspectiveProjection(45.f, static_cast<float>(Config::RESOLUTION_WIDTH) / static_cast<float>(Config::RESOLUTION_HEIGHT), 0.1f, 10.f);

    const VkFormat depthFormat = swapChain.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        _physicalDevice.getHandle(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    auto renderPassPtr = _logicalDevice.createRenderPass(surfaceFormat.format, depthFormat);

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

    if (vkCreateDescriptorSetLayout(_logicalDevice.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        std::cout << "failed to create descriptor set layout!" << std::endl;
        return 1;
    }

    avocado::vulkan::DescriptorSetLayoutPtr descriptorSetLayoutPtr = _logicalDevice.createObjectPointer(descriptorSetLayout);

    std::vector<VkDescriptorSet> descriptorSets;
    avocado::vulkan::DescriptorPoolPtr descriptorPool = _logicalDevice.createObjectPointer(_logicalDevice.createDescriptorPool(FRAMES_IN_FLIGHT));

    std::vector<avocado::vulkan::Buffer*> uniformBuffers = {&uniformBuffer, &uniformBufferForFrame2};

    // Create descriptor sets.
    std::vector<VkDescriptorSetLayout> layouts(FRAMES_IN_FLIGHT, descriptorSetLayoutPtr.get());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool.get();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(FRAMES_IN_FLIGHT);
    const VkResult ads = vkAllocateDescriptorSets(_logicalDevice.getHandle(), &allocInfo, descriptorSets.data());
    if (ads != VK_SUCCESS) {
        throw std::runtime_error("Can't allocate descriptor sets"s + std::to_string(static_cast<int>(ads)));
    }

    const std::vector<VkViewport> viewPorts { avocado::vulkan::Clipping::createViewport(0.f, 0.f, extent) };
    const std::vector<VkRect2D> scissors { avocado::vulkan::Clipping::createScissor(viewPorts.front()) };
    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder = preparePipeline(extent, layouts, viewPorts, scissors);
    avocado::vulkan::PipelinePtr graphicsPipeline = pipelineBuilder.createPipeline(renderPassPtr.get());
    if (pipelineBuilder.hasError()) {
        std::cout << "Can't create pipeline: " << pipelineBuilder.getErrorMessage() << std::endl;
        return 1;
    }

    swapChain.createDepthImage(extent.width, extent.height, _physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (swapChain.hasError()) {
        std::cout << "Error: can't create depth image (" << swapChain.getErrorMessage() << ")" << std::endl;
        return 1;
    }

    debugUtilsPtr->setObjectName(swapChain.getDepthImage(), "Depth image");

    avocado::vulkan::Queue graphicsQueue(_logicalDevice.getGraphicsQueue(0));
    debugUtilsPtr->setObjectName(graphicsQueue.getHandle(), "Graphics queue");

    avocado::vulkan::CommandPoolPtr commandPool = _logicalDevice.createObjectPointer(_logicalDevice.createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamily));
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create command pool: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::CommandBuffer> cmdBuffers = _logicalDevice.allocateCommandBuffers(FRAMES_IN_FLIGHT, commandPool.get(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    if (_logicalDevice.hasError()) {
        std::cout << "Can't allocate command buffers (" << _logicalDevice.getErrorMessage() << ")." << std::endl;
        return 1;
    }

    // Synchronization objects.
    std::array<avocado::vulkan::SemaphorePtr, FRAMES_IN_FLIGHT> imageAvailableSemaphores = {
        _logicalDevice.createObjectPointer(_logicalDevice.createSemaphore()),
        _logicalDevice.createObjectPointer(_logicalDevice.createSemaphore())};
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::array<avocado::vulkan::SemaphorePtr, FRAMES_IN_FLIGHT> renderFinishedSemaphores = {
        _logicalDevice.createObjectPointer(_logicalDevice.createSemaphore()),
        _logicalDevice.createObjectPointer(_logicalDevice.createSemaphore())};
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::array<avocado::vulkan::FencePtr, FRAMES_IN_FLIGHT> fences = {
        _logicalDevice.createObjectPointer(_logicalDevice.createFence()),
        _logicalDevice.createObjectPointer(_logicalDevice.createFence())};
    std::vector<VkFence> fenceHandles {fences[0].get(), fences[1].get()};
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create fence: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }


    if (swapChain.hasError()) {
        std::cout << "Can't get img index " << swapChain.getErrorMessage() << std::endl;
    }

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

    avocado::vulkan::Buffer imgTransferBuffer(imgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    imgTransferBuffer.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    imgTransferBuffer.fill(convertedSurface->pixels);
    imgTransferBuffer.bindMemory();

    convertedSurface.reset(); // Free resources.

    // Create image.
    avocado::vulkan::Image textureImage(_logicalDevice, imgW, imgH, VK_IMAGE_TYPE_2D);
    textureImage.setDepth(1);
    textureImage.setFormat(VK_FORMAT_R8G8B8A8_SRGB);
    textureImage.setMipLevels(1);
    textureImage.setImageTiling(VK_IMAGE_TILING_OPTIMAL);
    textureImage.setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    textureImage.setSampleCount(VK_SAMPLE_COUNT_1_BIT);
    textureImage.setArrayLayerCount(1);
    textureImage.setSharingMode(VK_SHARING_MODE_EXCLUSIVE);
    textureImage.create();
    if (textureImage.hasError()) {
        std::cout << "Image creation error: " << textureImage.getErrorMessage() << std::endl;
        return 1;
    }

    textureImage.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    textureImage.bindMemory();

    avocado::vulkan::ImageViewPtr textureImageView = _logicalDevice.createObjectPointer(swapChain.createImageView(textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));
    swapChain.createFramebuffers(renderPassPtr.get(), extent);

    transitionImageLayout(_logicalDevice, commandPool.get(), graphicsQueue, textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
    copyBufferToImage(_logicalDevice, commandPool.get(), graphicsQueue, imgTransferBuffer, textureImage, static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH));
    transitionImageLayout(_logicalDevice, commandPool.get(), graphicsQueue, textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

    avocado::vulkan::SamplerPtr textureSamplerPtr = _logicalDevice.createSampler(_physicalDevice);
    if (_logicalDevice.hasError()) {
        std::cout << "Error while creating sampler (" << _logicalDevice.getErrorMessage() << ")" << std::endl;
        return 1;
    }

    updateDescriptorSets(uniformBuffers, sizeof(UniformBufferObject), descriptorSets, textureImageView, textureSamplerPtr);
    VkBuffer vertexBufferHandle = vertexBuffer.getHandle();
    VkDeviceSize offset = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<VkPipelineStageFlags> flags = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    std::vector<VkSemaphore> waitSemaphores {imageAvailableSemaphores[0].get(), imageAvailableSemaphores[1].get()};
    std::vector<VkSemaphore> signalSemaphores {renderFinishedSemaphores[0].get(), renderFinishedSemaphores[1].get()};
    std::vector cmdBufferHandles = avocado::vulkan::getCommandBufferHandles(cmdBuffers);

    SDL_Event event;
    uint32_t imageIndex = 0;
    uint32_t currentFrame = 0;

    // Main loop.
    while (true) {
        if (SDL_PollEvent(&event)) {
            if ((event.type == SDL_QUIT) || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                break;
        }

        std::vector<VkFence> fenceToWait{fences[currentFrame].get()};
        _logicalDevice.waitForFences(fenceToWait, true);
        imageIndex = swapChain.acquireNextImage(imageAvailableSemaphores[currentFrame].get());
        _logicalDevice.resetFences(fenceToWait);

        // Update uniform buffer.
        const auto& currentTime = std::chrono::high_resolution_clock::now();
        const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        ubo.model = avocado::math::Mat4x4::createIdentityMatrix() * avocado::math::createRotationMatrix(time * 90.f, avocado::math::vec3f(0.0f, 0.0f, 1.0f));
        uniformBuffers[currentFrame]->fill(&ubo);

        avocado::vulkan::CommandBuffer commandBuffer = cmdBuffers[currentFrame];
        commandBuffer.reset(static_cast<VkCommandPoolResetFlagBits>(0));
        commandBuffer.begin();
                commandBuffer.setViewports(viewPorts);
                commandBuffer.setScissors(scissors);
                commandBuffer.bindVertexBuffers(0, 1, &vertexBufferHandle, &offset);
                commandBuffer.bindIndexBuffer(indexBuffer.getHandle(), 0, avocado::vulkan::toIndexType<decltype(indices)::value_type>());
                commandBuffer.bindPipeline(graphicsPipeline.get(), VK_PIPELINE_BIND_POINT_GRAPHICS);
                commandBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBuilder.getPipelineLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);

                commandBuffer.beginRenderPass(swapChain, renderPassPtr.get(), extent, {0, 0}, imageIndex);
                    commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                commandBuffer.endRenderPass();
        commandBuffer.end();

        auto submitInfo = graphicsQueue.createSubmitInfo(waitSemaphores[currentFrame], signalSemaphores[currentFrame], cmdBufferHandles[currentFrame], flags);
        graphicsQueue.submit(submitInfo, fenceToWait[0]);
        if (graphicsQueue.hasError()) {
            std::cout << "Can't submit graphics queue: " << graphicsQueue.getErrorMessage() << std::endl;
            break;
        }

        presentQueue.present(signalSemaphores[currentFrame], imageIndex, swapChainHandles[0]);
        currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
    } // Main loop.

    _logicalDevice.waitIdle();
    return 0;
}

