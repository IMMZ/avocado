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
#include <vulkan/states/colorblendstate.hpp>
#include <vulkan/states/dynamicstate.hpp>
#include <vulkan/states/inputasmstate.hpp>
#include <vulkan/states/multisamplestate.hpp>
#include <vulkan/states/rasterizationstate.hpp>
#include <vulkan/states/vertexinputstate.hpp>
#include <vulkan/states/viewportstate.hpp>
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

    if (!std::filesystem::exists(Config::SHADERS_PATH + "/triangle.frag.spv")) {
        std::cout << "File " << Config::SHADERS_PATH << "/triangle.frag.spv doesn't exist." << std::endl;
        return pipelineBuilder;
    }
    if (!std::filesystem::exists(Config::SHADERS_PATH + "/triangle.vert.spv")) {
        std::cout << "File " << Config::SHADERS_PATH << "/triangle.vert.spv doesn't exist." << std::endl;
        return pipelineBuilder;
    }

    { // Clear vector buffers immediatly after use.
        const std::vector<char> &fragBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.frag.spv");
        const std::vector<char> &vertBuf = avocado::utils::readFile(Config::SHADERS_PATH + "/triangle.vert.spv");
        pipelineBuilder.addVertexShaderModules({vertBuf});
        pipelineBuilder.addFragmentShaderModules({fragBuf});
    }

    auto dynState = std::make_unique<avocado::vulkan::DynamicState>(std::vector<VkDynamicState>{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
    pipelineBuilder.setDynamicState(std::move(dynState));

    auto inAsmState = std::make_unique<avocado::vulkan::InputAsmState>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
    pipelineBuilder.setInputAsmState(std::move(inAsmState));

    auto rastState = std::make_unique<avocado::vulkan::RasterizationState>();
    rastState->setDepthBiasEnabled(VK_FALSE);
    rastState->setPolygonMode(VK_POLYGON_MODE_FILL);
    rastState->setCullMode(VK_CULL_MODE_BACK_BIT);
    rastState->setFrontFace(VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.setRasterizationState(std::move(rastState));

    auto multisampleState = std::make_unique<avocado::vulkan::MultisampleState>();
    multisampleState->setRasterizationSamples(VK_SAMPLE_COUNT_1_BIT);
    pipelineBuilder.setMultisampleState(std::move(multisampleState));

    auto colorBlendState = std::make_unique<avocado::vulkan::ColorBlendState>();
    colorBlendState->setLogicOp(VK_LOGIC_OP_COPY);
    colorBlendState->addAttachment({
            VK_TRUE // Blend enabled.
            , VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD // Color blend factor.
            , VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD // Alpha blend factor.
            , VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT // Color write mask.
            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});
    pipelineBuilder.setColorBlendState(std::move(colorBlendState));

    auto vertexInState = std::make_unique<avocado::vulkan::VertexInputState>();
    vertexInState->addAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(avocado::Vertex, position));
    vertexInState->addAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(avocado::Vertex, color));
    vertexInState->addAttributeDescription(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(avocado::Vertex, textureCoordinate));
    vertexInState->addBindingDescription(0, sizeof(avocado::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineBuilder.setVertexInputState(std::move(vertexInState));

    auto viewportState = std::make_unique<avocado::vulkan::ViewportState>(viewPorts, scissors);
    pipelineBuilder.setViewportState(std::move(viewportState));

    pipelineBuilder.setDSLayouts(layouts); // todo rename this method

    return pipelineBuilder;
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
    const VkSurfaceFormatKHR surfaceFormat = surface.findFormat(
        avocado::vulkan::Format::B8G8R8A8SRGB,
        avocado::vulkan::Surface::ColorSpace::SRGBNonlinearKHR);
    avocado::vulkan::Swapchain swapChain = createSwapchain(surface, surfaceFormat, extent, queueFamilies);


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
        _logicalDevice, _physicalDevice);

    if (vertexBuffer.hasError()) {
        std::cout << "Can't create vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (vertexBuffer.hasError()) {
        std::cout << "Can't allocate memory on vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.fill(quad.data());
    vertexBuffer.bindMemory();

    avocado::vulkan::Buffer indexBuffer(indicesSizeBytes,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
        VK_SHARING_MODE_EXCLUSIVE, _logicalDevice, _physicalDevice);
    if (indexBuffer.hasError()) {
        std::cout << "Can't create index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }
    indexBuffer.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
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

    avocado::vulkan::Buffer uniformBuffer(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice, _physicalDevice);
    uniformBuffer.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    avocado::vulkan::Buffer uniformBuffer1(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice, _physicalDevice);
    uniformBuffer1.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBuffer1.bindMemory();

    UniformBufferObject ubo{};
    ubo.view = avocado::math::lookAt(avocado::math::vec3f(0.f, 0.f, 2.f), avocado::math::vec3f(0.f, 0.f, 0.f), avocado::math::vec3f(0.f, 1.f, 0.f));
    ubo.proj = avocado::math::perspectiveProjection(45.f, static_cast<float>(Config::RESOLUTION_WIDTH) / static_cast<float>(Config::RESOLUTION_HEIGHT), 0.1f, 10.f);
    uniformBuffer.fill(&ubo);
    uniformBuffer.bindMemory();

    auto renderPassPtr = _logicalDevice.createRenderPass(surfaceFormat.format);


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
    avocado::vulkan::DescriptorPoolPtr descriptorPool = _logicalDevice.createObjectPointer(_logicalDevice.createDescriptorPool());

    std::array<avocado::vulkan::Buffer*, 2> uniformBuffers = {&uniformBuffer, &uniformBuffer1};

    // Create descriptor sets.
    std::vector<VkDescriptorSetLayout> layouts(2, descriptorSetLayoutPtr.get());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool.get();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(2);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(2);
    const VkResult ads = vkAllocateDescriptorSets(_logicalDevice.getHandle(), &allocInfo, descriptorSets.data());
    if (ads != VK_SUCCESS) {
        throw std::runtime_error("Can't allocate descriptor sets"s + std::to_string(static_cast<int>(ads)));
    }

    const std::vector<VkViewport> viewPorts { avocado::vulkan::Clipping::createViewport(0.f, 0.f, extent) };
    const std::vector<VkRect2D> scissors { avocado::vulkan::Clipping::createScissor(viewPorts.front()) };
    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder = preparePipeline(extent, layouts, viewPorts, scissors);
    avocado::vulkan::PipelinePtr graphicsPipeline = pipelineBuilder.buildPipeline(renderPassPtr.get());

    if (graphicsPipeline == nullptr) {
        std::cout << "Error: invalid pipeline" << std::endl;
        return 1;
    }

    avocado::vulkan::CommandPoolPtr commandPool = _logicalDevice.createObjectPointer(_logicalDevice.createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamily));
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create command pool: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::CommandBuffer> cmdBuffers = _logicalDevice.allocateCommandBuffers(1, commandPool.get(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    if (_logicalDevice.hasError()) {
        std::cout << "Can't allocate command buffers (" << _logicalDevice.getErrorMessage() << ")." << std::endl;
        return 1;
    }

    avocado::vulkan::CommandBuffer commandBuffer = cmdBuffers.front();

    // Synchronization objects.
    avocado::vulkan::SemaphorePtr imageAvailableSemaphore = _logicalDevice.createObjectPointer(_logicalDevice.createSemaphore());
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::SemaphorePtr renderFinishedSemaphore = _logicalDevice.createObjectPointer(_logicalDevice.createSemaphore());
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    avocado::vulkan::FencePtr fence = _logicalDevice.createObjectPointer(_logicalDevice.createFence());
    std::vector<VkFence> fences {fence.get()};
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create fence: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    const std::vector<VkSemaphore> waitSemaphores {imageAvailableSemaphore.get()};
    const std::vector<VkSemaphore> signalSemaphores {renderFinishedSemaphore.get()};

    avocado::vulkan::Queue graphicsQueue(_logicalDevice.getGraphicsQueue(0));
    debugUtilsPtr->setObjectName(graphicsQueue.getHandle(), "Graphics queue");

    std::vector cmdBufferHandles = avocado::vulkan::getCommandBufferHandles(cmdBuffers);

    if (swapChain.hasError()) {
        std::cout << "Can't get img index " << swapChain.getErrorMessage() << std::endl;
    }

    std::vector imageIndices {swapChain.acquireNextImage(imageAvailableSemaphore.get())};
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

    avocado::vulkan::Buffer imgTransferBuffer(imgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, _logicalDevice, _physicalDevice);
    imgTransferBuffer.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    imgTransferBuffer.fill(convertedSurface->pixels);
    imgTransferBuffer.bindMemory();
    convertedSurface.reset();

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

    VkDeviceMemory textureImageMemory;

    // Setup image memory. Allocate and bind it.
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_logicalDevice.getHandle(), textureImage.getHandle(), &memRequirements);

    VkMemoryAllocateInfo imageMemAI{}; FILL_S_TYPE(imageMemAI);
    imageMemAI.allocationSize = memRequirements.size;

    // todo Do the func for buffer & image for found type index.
    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice.getHandle(), &memProps);

    const uint32_t typeFilter = memRequirements.memoryTypeBits;

    uint32_t foundType = 0;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            foundType = i;
            break;
        }
    }

    imageMemAI.memoryTypeIndex = foundType;
    const VkResult v2 = vkAllocateMemory(_logicalDevice.getHandle(), &imageMemAI, nullptr, &textureImageMemory);
    if (v2 != VK_SUCCESS) {
        std::cout << "vkAllocateMemory error" << std::endl;
        return 1;
    }

    const VkResult v3 = vkBindImageMemory(_logicalDevice.getHandle(), textureImage.getHandle(), textureImageMemory, 0);
    if (v3 != VK_SUCCESS) {
        std::cout << "vkBindImageMemory error" << std::endl;
        return 1;
    }

    avocado::vulkan::ImageViewPtr textureImageView = _logicalDevice.createObjectPointer(swapChain.createImageView(textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB));
    swapChain.createFramebuffers(renderPassPtr.get(), extent);

    transitionImageLayout(_logicalDevice, commandPool.get(), graphicsQueue, textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(_logicalDevice, commandPool.get(), graphicsQueue, imgTransferBuffer.getHandle(), textureImage.getHandle(), static_cast<uint32_t>(imgW), static_cast<uint32_t>(imgH));
    transitionImageLayout(_logicalDevice, commandPool.get(), graphicsQueue, textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.anisotropyEnable = VK_FALSE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_physicalDevice.getHandle(), &properties);
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
    vkCreateSampler(_logicalDevice.getHandle(), &samplerInfo, nullptr, &textureSampler);
    avocado::vulkan::SamplerPtr textureSamplerPtr = _logicalDevice.createObjectPointer(textureSampler);

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


        vkUpdateDescriptorSets(_logicalDevice.getHandle(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }

    VkBuffer vertexBufferHandle = vertexBuffer.getHandle();
    VkDeviceSize offset = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    // Main loop.
    while (true) {
        if (SDL_PollEvent(&event)) {
            if ((event.type == SDL_QUIT) || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                break;
        }

        _logicalDevice.waitForFences(fences, true);
        _logicalDevice.resetFences(fences);

        // Update uniform buffer.
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        ubo.model = avocado::math::Mat4x4::createIdentityMatrix() * avocado::math::createRotationMatrix(time * 90.f, avocado::math::vec3f(0.0f, 0.0f, 1.0f));
        uniformBuffer.fill(&ubo);

        commandBuffer.reset(avocado::vulkan::CommandBuffer::ResetFlags::NoFlags);
        commandBuffer.begin();
        commandBuffer.beginRenderPass(swapChain, renderPassPtr.get(), extent, {0, 0}, imageIndices.front());

        commandBuffer.setViewports(viewPorts);
        commandBuffer.setScissors(scissors);
        commandBuffer.bindVertexBuffers(0, 1, &vertexBufferHandle, &offset);
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
    } // Main loop.

    _logicalDevice.waitIdle();

    // Destroy resources.
    vkFreeMemory(_logicalDevice.getHandle(), textureImageMemory, nullptr);
    pipelineBuilder.destroyPipeline();

    return 0;
}
