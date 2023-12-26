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

void createDescriptorSetLayout() {
}

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

    physicalDevice.initQueueFamilies(surface);
    if (physicalDevice.hasError()) {
        std::cout << "Can't get queue families: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }
    const avocado::vulkan::QueueFamily graphicsQueueFamily = physicalDevice.getGraphicsQueueFamily();
    const avocado::vulkan::QueueFamily transferQueueFamily = physicalDevice.getTransferQueueFamily();
    const avocado::vulkan::QueueFamily presentQueueFamily = physicalDevice.getPresentQueueFamily();

    if (surface.hasError()) {
        std::cout << "Can't get present queue family index: " << surface.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<avocado::vulkan::QueueFamily> queueFamilies {graphicsQueueFamily, transferQueueFamily, presentQueueFamily};
    avocado::utils::makeUniqueContainer(queueFamilies);

    avocado::vulkan::LogicalDevice logicalDevice = physicalDevice.createLogicalDevice(queueFamilies, physExtensions, instanceLayers, 1, 1.0f);
    if (physicalDevice.hasError()) {
        std::cerr << "Can't create logical device: " << physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    auto debugUtilsPtr = logicalDevice.createDebugUtils();

    // New code.
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


    VkDescriptorSetLayout layoutPtr = logicalDevice.createDescriptorSetLayout({layoutBinding});
    if (logicalDevice.hasError()) {
        std::cout << "Error creating descriptor set layout: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.setLayoutCount = 1;
    const auto layoutRawPtr = layoutPtr; // todo duplicate, remove!
    pipelineLayoutCI.pSetLayouts = &layoutRawPtr;

    VkPipelineLayout newPipelineLayout = VK_NULL_HANDLE;
    const VkResult rr = vkCreatePipelineLayout(logicalDevice.getHandle(), &pipelineLayoutCI, nullptr, &newPipelineLayout);
    if (rr != VK_SUCCESS) {
        std::cout << "Couldn't create pipeline layout and error is " << static_cast<int>(rr) << std::endl;
    } else {
        debugUtilsPtr->setObjectName(newPipelineLayout, "New pipeline layout");
        std::cout << "New pipeline layout created OK" << std::endl;
    }

    // New code end
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

    // todo move to pipeline class.
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

    // Building a pipeline.
    avocado::vulkan::GraphicsPipelineBuilder pipelineBuilder(logicalDevice.getHandle());

    avocado::vulkan::DynamicState dynState({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
    pipelineBuilder.setDynamicState(dynState);

    avocado::vulkan::InputAsmState inAsmState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
    pipelineBuilder.setInputAsmState(inAsmState);

    avocado::vulkan::RasterizationState rastState;
    rastState.setPolygonMode(VK_POLYGON_MODE_FILL);
    rastState.setCullMode(VK_CULL_MODE_BACK_BIT);
    rastState.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
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
    vertexInState.addBindingDescription(0, sizeof(avocado::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineBuilder.setVertexInputState(vertexInState);

    constexpr std::array<avocado::Vertex, 4> quad = {{
        /* { pos, color } */
        {avocado::math::vec2f(-.5f, -.5f), avocado::math::vec3f(1.f, 0.f, 0.f)},
        {avocado::math::vec2f( .5f, -.5f), avocado::math::vec3f(0.f, 1.f, 0.f)},
        {avocado::math::vec2f( .5f,  .5f), avocado::math::vec3f(0.f, 0.f, 1.f)},
        {avocado::math::vec2f(-.5f,  .5f), avocado::math::vec3f(1.f, 1.f, 1.f)}
    }};
    constexpr VkDeviceSize verticesSizeBytes = sizeof(decltype(quad)::value_type) * quad.size();
    constexpr std::array<uint16_t, 6> indices {0, 1, 2, 2, 3, 0};
    constexpr size_t indicesSizeBytes = indices.size() * sizeof(decltype(indices)::value_type);

    // Create transfer and vertex buffers.
    // Transfer buffer is used to transfer both vertices and indices.
    avocado::vulkan::Buffer transferBuf(verticesSizeBytes + indicesSizeBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE,
        logicalDevice, physicalDevice);
    if (transferBuf.hasError()) {
        std::cout << "Can't create transfer buf: " << transferBuf.getErrorMessage() << std::endl;
        return 1;
    }

    transferBuf.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (transferBuf.hasError()) {
        std::cout << "Can't allocate memory on transfer buf: " << transferBuf.getErrorMessage() << std::endl;
        return 1;
    }

    transferBuf.fill(quad.data(), verticesSizeBytes);
    if (transferBuf.hasError()) {
        std::cout << "Can't fill memory on transfer buf: " << transferBuf.getErrorMessage() << std::endl;
        return 1;
    }

    transferBuf.bindMemory();

    avocado::vulkan::Buffer vertexBuffer(verticesSizeBytes,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
        VK_SHARING_MODE_EXCLUSIVE,
        logicalDevice, physicalDevice);

    if (vertexBuffer.hasError()) {
        std::cout << "Can't create vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vertexBuffer.hasError()) {
        std::cout << "Can't allocate memory on vertex buf: " << vertexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    vertexBuffer.bindMemory();

    avocado::vulkan::Buffer indexBuffer(indicesSizeBytes,
        static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
        VK_SHARING_MODE_EXCLUSIVE, logicalDevice, physicalDevice);
    if (indexBuffer.hasError()) {
        std::cout << "Can't create index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }
    indexBuffer.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (indexBuffer.hasError()) {
        std::cout << "Can't allocate memory on index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    indexBuffer.bindMemory();
    if (indexBuffer.hasError()) {
        std::cout << "Can't bind memory of index buffer: " << indexBuffer.getErrorMessage() << std::endl;
        return 1;
    }

    struct UniformBufferObject {
        avocado::math::Mat4x4 model;
        avocado::math::Mat4x4 proj;
        avocado::math::Mat4x4 view;
    };

    avocado::vulkan::Buffer uniformBuffer(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, logicalDevice, physicalDevice);
    uniformBuffer.allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBuffer.bindMemory();

    auto renderPassPtr = logicalDevice.createRenderPass(surfaceFormat.format);
    const std::vector<VkViewport> viewPorts { avocado::vulkan::Clipping::createViewport(0.f, 0.f, extent) };
    const std::vector<VkRect2D> scissors { avocado::vulkan::Clipping::createScissor(viewPorts.front()) };
    avocado::vulkan::ViewportState viewportState(viewPorts, scissors);
    pipelineBuilder.setViewportState(viewportState);

    avocado::vulkan::GraphicsPipelineBuilder::PipelineUniquePtr graphicsPipeline = pipelineBuilder.buildPipeline(pipelineShaderStageCIs, renderPassPtr.get());

    if (graphicsPipeline == nullptr) {
        std::cout << "Error: invalid pipeline" << std::endl;
        return 1;
    }

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
    commandBuffer.bindPipeline(graphicsPipeline.get(), avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
    // Binding vertex buffer.
    VkBuffer vertexBuffers[] {vertexBuffer.getHandle()};
    VkDeviceSize offsets[] {0};

    //commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    //commandBuffer.bindIndexBuffer(indexBuffer.getHandle(), 0, avocado::vulkan::toIndexType<decltype(indices)::value_type>());
    
    commandBuffer.setViewports(viewPorts);
    commandBuffer.setScissors(scissors);
    commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
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

    const std::vector<VkSemaphore> waitSemaphores {imageAvailableSemaphore};
    const std::vector<VkSemaphore> signalSemaphores {renderFinishedSemaphore};

    avocado::vulkan::Queue graphicsQueue(logicalDevice.getGraphicsQueue(0));
    debugUtilsPtr->setObjectName(graphicsQueue.getHandle(), "Graphics queue");

    avocado::vulkan::Queue transferQueue(logicalDevice.getTransferQueue(0));

    std::vector cmdBufferHandles = avocado::vulkan::getCommandBufferHandles(cmdBuffers);

    std::vector<avocado::vulkan::CommandBuffer> newCmdBufs = logicalDevice.allocateCommandBuffers(1, commandPool, avocado::vulkan::LogicalDevice::CommandBufferLevel::Primary);
    if (logicalDevice.hasError()) {
        std::cout << "Can't allocate new command buffers: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }
    avocado::vulkan::CommandBuffer newCmdBuf = newCmdBufs.front();
    debugUtilsPtr->setObjectName(newCmdBuf.getHandle(), "Command buf for buffer copying");
    if (logicalDevice.hasError()) {
        std::cout << "Logical device error: " << logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    newCmdBuf.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    if (newCmdBuf.hasError()) {
        std::cout << "Can't begin new cmdbuf: " << newCmdBuf.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<VkBufferCopy> vertexCopyRegions(1);
    vertexCopyRegions.front().size = verticesSizeBytes;
    newCmdBuf.copyBuffer(transferBuf, vertexBuffer, vertexCopyRegions);

    transferBuf.fill(indices.data(), indicesSizeBytes, verticesSizeBytes);
    if (transferBuf.hasError()) {
        std::cout << "Error re-filling the buffer: " << transferBuf.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<VkBufferCopy> indexCopyRegions(1);
    indexCopyRegions.front().size = indicesSizeBytes;
    indexCopyRegions.front().srcOffset = verticesSizeBytes;
    newCmdBuf.copyBuffer(transferBuf, indexBuffer, indexCopyRegions);
    newCmdBuf.end();
    if (newCmdBuf.hasError()) {
        std::cout << "Can't end new cmdbuf: " << newCmdBuf.getErrorMessage() << std::endl;
        return 1;
    }


    std::vector newCmdBufHandles = avocado::vulkan::getCommandBufferHandles(newCmdBufs);
    auto subInfo = transferQueue.createSubmitInfo({}, {}, newCmdBufHandles, {});
    transferQueue.submit(subInfo);
    if (transferQueue.hasError()) {
        std::cout << "Can't submit gr queue: " << transferQueue.getErrorMessage() << std::endl;
        return 1;
    }
    transferQueue.waitIdle();
    if (transferQueue.hasError()) {
        std::cout << "Can't wait idle gr queue: " << transferQueue.getErrorMessage() << std::endl;
        return 1;
    }

    const uint32_t imgIndex = swapchain.acquireNextImage(imageAvailableSemaphore);
    if (swapchain.hasError()) {
        std::cout << "Can't get img index " << swapchain.getErrorMessage() << std::endl;
    }

    std::vector imageIndices {imgIndex};

    std::vector<VkPipelineStageFlags> flags = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    auto submitInfo = graphicsQueue.createSubmitInfo(waitSemaphores, signalSemaphores, cmdBufferHandles, flags);
    SDL_Event event;
    std::vector<VkSwapchainKHR> swapchainHandles(swapchains.size());
    std::transform(swapchains.begin(), swapchains.end(), swapchainHandles.begin(), [](avocado::vulkan::Swapchain &swapchain) { return swapchain.getHandle(); });

    VkDescriptorPoolSize descriptorPoolSize{};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo dPoolCI{};
    dPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dPoolCI.poolSizeCount = 1;
    dPoolCI.pPoolSizes = &descriptorPoolSize;
    dPoolCI.maxSets = 1;

    VkDescriptorPool descriptorPool;
    const VkResult cdp = vkCreateDescriptorPool(logicalDevice.getHandle(), &dPoolCI, nullptr, &descriptorPool);
    if (cdp != VK_SUCCESS) {
        std::cout << "Can't create descriptor pool: " << static_cast<int>(cdp) << std::endl;
        return 1;
    }

    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layoutRawPtr;

    VkDescriptorSet dSet;
    const VkResult ads = vkAllocateDescriptorSets(logicalDevice.getHandle(), &allocInfo, &dSet);
    if (ads != VK_SUCCESS) {
        std::cout << "Can't allocate descripto sets: " << static_cast<int>(ads) << std::endl;
        return 1;
    }

    VkDescriptorBufferInfo dBufInfo{};
    dBufInfo.buffer = uniformBuffer.getHandle();
    dBufInfo.offset = 0;
    dBufInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = dSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &dBufInfo;


    vkUpdateDescriptorSets(logicalDevice.getHandle(), 1, &descriptorWrite, 0, nullptr);


    auto startTime = std::chrono::high_resolution_clock::now();
    // Main loop.
    while (true) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;
        }

        logicalDevice.waitForFences(fences, true, std::numeric_limits<uint64_t>::max());
        logicalDevice.resetFences(fences);

        // Update uniform buffer.

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        UniformBufferObject ubo{};
        ubo.model = avocado::math::createRotationMatrix(time * 90.f, avocado::math::vec3f(0.0f, 0.0f, 0.0f));
        ubo.view = avocado::math::lookAt(avocado::math::vec3f(2.f, 2.f, 2.f), avocado::math::vec3f(0.f, 0.f, 0.f), avocado::math::vec3f(0.f, 0.f, 1.f));
        ubo.proj =avocado::math::perspectiveProjection(avocado::math::toRadians(45.f), 800.f / 600.f, 0.1f, 10.f);
        uniformBuffer.fill(&ubo, sizeof(ubo));
        
        vkUpdateDescriptorSets(logicalDevice.getHandle(), 1, &descriptorWrite, 0, nullptr);

        commandBuffer.reset(avocado::vulkan::CommandBuffer::ResetFlags::NoFlags);
        commandBuffer.begin();
        commandBuffer.beginRenderPass(swapchain, renderPassPtr.get(), extent, {0, 0}, imageIndices.front());

        commandBuffer.setViewports(viewPorts);
        commandBuffer.setScissors(scissors);
        //commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(indexBuffer.getHandle(), 0, avocado::vulkan::toIndexType<decltype(indices)::value_type>());
        commandBuffer.bindPipeline(graphicsPipeline.get(), avocado::vulkan::CommandBuffer::PipelineBindPoint::Graphics);
        vkCmdBindDescriptorSets(commandBuffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, newPipelineLayout, 0, 1, &dSet, 0, nullptr);
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();

        graphicsQueue.submit(submitInfo, fences.front());
        if (graphicsQueue.hasError()) {
            std::cout << "Can't submit graphics queue: " << graphicsQueue.getErrorMessage() << std::endl;
        }

        presentQueue.present(signalSemaphores, imageIndices, swapchainHandles);
        imageIndices[0] = swapchain.acquireNextImage(imageAvailableSemaphore);
    }

    logicalDevice.waitIdle();

    // Destroy resources.
    vkDestroyDescriptorSetLayout(logicalDevice.getHandle(), layoutPtr, nullptr);
    vkDestroySemaphore(logicalDevice.getHandle(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(logicalDevice.getHandle(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(logicalDevice.getHandle(), fences.front(), nullptr);
    vkDestroyCommandPool(logicalDevice.getHandle(), commandPool, nullptr);

    return 0;
}
