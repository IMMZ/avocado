#include <ostream>

#include "application.hpp"

#include "gameconfig.hpp"
#include "vertex.hpp"
#include "utils.hpp"

#include <scene.hpp>
#include <os/osutils.hpp>
#include <vulkan/commandbuffer.hpp>
#include <vulkan/graphicspipeline.hpp>

#include <math/functions.hpp>
#include <math/matrix.hpp>
#include <math/vecn.hpp>

#include <vulkan/buffer.hpp>
#include <vulkan/clipping.hpp>
#include <vulkan/commandpool.hpp>
#include <vulkan/debugutils.hpp>
#include <vulkan/descriptorsetpool.hpp>
#include <vulkan/image.hpp>
#include <vulkan/logicaldevice.hpp>
#include <vulkan/pointertypes.hpp>
#include <vulkan/queuemanager.hpp>
#include <vulkan/scenetextures.hpp>
#include <vulkan/shaderstorage.hpp>
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

using namespace avocado;

void Application::createInstance(SDL_Window &window, const std::vector<std::string> &instanceLayers) {
    const bool areLayersSupported = _vulkan.areLayersSupported(instanceLayers);
    if (!areLayersSupported) {
        std::cerr << _vulkan.getErrorMessage() << std::endl;
        return;
    }

    std::vector<std::string> instanceExtensions = _vulkan.getExtensionNamesForSDLSurface(&window);
    if constexpr (core::isDebugBuild())
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (_vulkan.hasError()) {
        std::cerr << _vulkan.getErrorMessage() << std::endl;
        return;
    }

    constexpr vulkan::VulkanInstanceInfo vulkanInfo {
        GameConfig::GAME_NAME,
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
    std::vector<vulkan::PhysicalDevice> physicalDevices = _vulkan.getPhysicalDevices();
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

vulkan::Swapchain Application::createSwapchain(vulkan::Surface &surface, const VkSurfaceFormatKHR surfaceFormat, const VkExtent2D extent,
    const std::vector<vulkan::QueueFamily> &queueFamilies) {
    vulkan::Swapchain swapChain(_logicalDevice);
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
        GameConfig::GAME_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        GameConfig::RESOLUTION_WIDTH, GameConfig::RESOLUTION_HEIGHT,
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
vulkan::GraphicsPipelineBuilder Application::preparePipeline(const VkExtent2D extent,
    const std::vector<VkDescriptorSetLayout> &layouts, const std::vector<VkViewport> &viewPorts,
    const std::vector<VkRect2D> &scissors) {
    vulkan::GraphicsPipelineBuilder pipelineBuilder(_logicalDevice);
    pipelineBuilder.setDynamicStates({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
    pipelineBuilder.createDynamicState();

    VkPipelineInputAssemblyStateCreateInfo &inAsmState = pipelineBuilder.createInputAssmeblyState();
    inAsmState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo &rastState = pipelineBuilder.createRasterizationState();
    rastState.depthClampEnable = VK_FALSE;
    rastState.rasterizerDiscardEnable = VK_FALSE;
    rastState.depthBiasEnable = VK_FALSE;
    rastState.polygonMode = VK_POLYGON_MODE_FILL;
    rastState.cullMode = VK_CULL_MODE_FRONT_BIT;
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
    pipelineBuilder.addAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
    pipelineBuilder.addAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
    pipelineBuilder.addAttributeDescription(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, textureCoordinate));

    pipelineBuilder.addBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

    VkPipelineViewportStateCreateInfo &viewportState = pipelineBuilder.createViewportState();
    pipelineBuilder.setViewPorts(viewPorts);
    pipelineBuilder.setScissors(scissors);
    pipelineBuilder.setDescriptorSetLayouts(layouts);
    return pipelineBuilder;
}

int Application::run() {
    // Check resources.
    // todo The variants below are for tests. Each of them should work properly.
    //const std::string modelFile = avocado::os::getExecutablePath() + "/assets/models/non_textured_cube.glb";
    //const std::string modelFile = avocado::os::getExecutablePath() + "/assets/models/textured_cube.gltf";
    //const std::string modelFile = avocado::os::getExecutablePath() + "/assets/models/two_non_textured_cubes.glb";
    const std::string modelFile = avocado::os::getExecutablePath() + "/assets/models/scene1/scene.gltf";
    //const std::string modelFile = avocado::os::getExecutablePath() + "/assets/models/two_non_textured_cubes.gltf";
    if (!std::filesystem::exists(modelFile)) {
        std::cout << "File " << modelFile << " doesn't exist" << std::endl;
        return 1;
    }

    if (!std::filesystem::exists(GameConfig::getShadersPath())) {
        std::cout << "Directory " << GameConfig::getShadersPath() << " doesn't exist" << std::endl;
        return 1;
    }

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

    vulkan::Surface surface = _vulkan.createSurface(sdlWindow.get(), _physicalDevice);
    if (_vulkan.hasError()) {
        std::cerr << "Can't create surface: " << _vulkan.getErrorMessage() << std::endl;
        return 1;
    }

    _physicalDevice.initQueueFamilies(surface);
    if (_physicalDevice.hasError()) {
        std::cout << "Can't get queue families: " << _physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }
    const vulkan::QueueFamily graphicsQueueFamily = _physicalDevice.getGraphicsQueueFamily();
    const vulkan::QueueFamily presentQueueFamily = _physicalDevice.getPresentQueueFamily();

    if (surface.hasError()) {
        std::cout << "Can't get present queue family index: " << surface.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector queueFamilies {graphicsQueueFamily, presentQueueFamily};
    utils::makeUniqueContainer(queueFamilies);

    _logicalDevice = _physicalDevice.createLogicalDevice(queueFamilies, physExtensions, instanceLayers, 1, 1.0f);
    if (_physicalDevice.hasError()) {
        std::cerr << "Can't create logical device: " << _physicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    auto debugUtilsPtr = _logicalDevice.createDebugUtils();

    vulkan::QueueManager queueManager(_logicalDevice, graphicsQueueFamily);

    vulkan::Queue presentQueue = _logicalDevice.getPresentQueue(0);

    VkExtent2D extent = surface.getCapabilities(sdlWindow.get());

    // Clamp width and height to fit into capabilities.
    extent.width = std::clamp(extent.width, surface.getMinExtentW(), surface.getMaxExtentW());
    extent.height = std::clamp(extent.height, surface.getMinExtentH(), surface.getMaxExtentH());

    // Creating swapchain.
    const VkSurfaceFormatKHR surfaceFormat = surface.findFormat(VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    vulkan::Swapchain swapChain = createSwapchain(surface, surfaceFormat, extent, queueFamilies);

    vulkan::CommandPool commandPool(_logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamily);
    if (commandPool.hasError()) {
        std::cout << "Can't create command pool: " << commandPool.getErrorMessage() << std::endl;
        return 1;
    }

    commandPool.allocateBuffers(FRAMES_IN_FLIGHT + 3, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    if (commandPool.hasError()) {
        std::cout << "Can't allocate command buffers: " << commandPool.getErrorMessage() << std::endl;
        return 1;
    }

    vulkan::Queue graphicsQueue(_logicalDevice.getGraphicsQueue(0));
    debugUtilsPtr->setObjectName(graphicsQueue.getHandle(), "Graphics queue");

    core::SceneManager sceneManager;
    sceneManager.load(modelFile);
    vulkan::SceneTextures sceneTextures(_physicalDevice, _logicalDevice);
    sceneTextures.load(sceneManager, swapChain, queueManager);

    auto vertexBuffer = sceneManager.formVertexBuffer(_physicalDevice, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    auto indexBuffer = sceneManager.formIndexBuffer(_physicalDevice, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);

    struct UniformBufferObject {
        alignas(16) math::Mat4x4 model;
        alignas(16) math::Mat4x4 view;
        alignas(16) math::Mat4x4 proj;
    };

    vulkan::Buffer uniformBuffer(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    uniformBuffer.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBuffer.bindMemory();

    vulkan::Buffer uniformBufferForFrame2(sizeof(UniformBufferObject), static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    uniformBufferForFrame2.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBufferForFrame2.bindMemory();

    UniformBufferObject ubo{};
    ubo.view = math::lookAt(math::vec3f(0.f, 0.f, -8.f), math::vec3f(0.f, 0.f, 0.f), math::vec3f(0.f, 1.f, 0.f));
    //ubo.view = math::lookAt(math::vec3f(5.f, 0.f, -8.f), math::vec3f(0.f, 0.f, 0.f), math::vec3f(0.f, 1.f, 0.f));
    ubo.proj = math::perspectiveProjection(45.f, static_cast<float>(GameConfig::RESOLUTION_WIDTH) / static_cast<float>(GameConfig::RESOLUTION_HEIGHT), 0.1f, 50.f);
    ubo.model = math::Mat4x4::createIdentityMatrix();
    ubo.model[1][1] = -1.f; // Flip Y axis.
    const VkFormat depthFormat = swapChain.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        _physicalDevice.getHandle(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    const auto renderPassPtr = _logicalDevice.createRenderPass(surfaceFormat.format, depthFormat);

    vulkan::DescriptorSetPool descriptorSetPool = sceneTextures.exportToDescriptorSet();

    // Create descriptor set layout.
    std::vector<vulkan::Buffer*> uniformBuffers = {&uniformBuffer, &uniformBufferForFrame2};
    constexpr uint32_t BUFFER_BINDING = 0;
    const size_t writeIndexForFirstBuffer = descriptorSetPool.addBuffer(0, BUFFER_BINDING, *(uniformBuffers[0]), sizeof(UniformBufferObject));
    const size_t writeIndexForSecondBuffer = descriptorSetPool.addBuffer(1, BUFFER_BINDING, *(uniformBuffers[1]), sizeof(UniformBufferObject));

    const std::vector<VkViewport> viewPorts { vulkan::Clipping::createViewport(0.f, 0.f, extent) };
    const std::vector<VkRect2D> scissors { vulkan::Clipping::createScissor(viewPorts.front()) };
    vulkan::ShaderStorage shaderStorage(_logicalDevice);
    shaderStorage.loadShaders(GameConfig::getShadersPath());

    vulkan::GraphicsPipelineBuilder pipelineBuilder = preparePipeline(extent, descriptorSetPool.getLayouts(), viewPorts, scissors);

    {
      const std::vector<VkPipelineShaderStageCreateInfo> &redColorShaders = shaderStorage.createStageCIs({
          vulkan::ShaderIdConst::VERTEX_PROCESS_INPUT,
          vulkan::ShaderIdConst::FRAGMENT_RED_COLOR });
      const std::vector<VkPipelineShaderStageCreateInfo> &oneTextureShaders = shaderStorage.createStageCIs({
          vulkan::ShaderIdConst::VERTEX_PROCESS_INPUT,
          vulkan::ShaderIdConst::FRAGMENT_ONE_TEXTURE });

      pipelineBuilder.bindShaderModules(oneTextureShaders);
    }

    vulkan::PipelinePtr graphicsPipeline = pipelineBuilder.createPipeline(renderPassPtr.get());
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

    // Synchronization objects.
    const std::array<vulkan::SemaphorePtr, FRAMES_IN_FLIGHT> imageAvailableSemaphores = {_logicalDevice.createSemaphore(), _logicalDevice.createSemaphore()};
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    const std::array<vulkan::SemaphorePtr, FRAMES_IN_FLIGHT> renderFinishedSemaphores = {_logicalDevice.createSemaphore(), _logicalDevice.createSemaphore()};
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create semaphore: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    const std::array<vulkan::FencePtr, FRAMES_IN_FLIGHT> fences = {_logicalDevice.createFence(), _logicalDevice.createFence()};
    std::vector<VkFence> fenceHandles {fences[0].get(), fences[1].get()};
    if (_logicalDevice.hasError()) {
        std::cout << "Can't create fence: " << _logicalDevice.getErrorMessage() << std::endl;
        return 1;
    }

    std::vector<VkSwapchainKHR> swapChainHandles{swapChain.getHandle()};

    swapChain.createFramebuffers(renderPassPtr.get(), extent);

    descriptorSetPool.update();

    VkBuffer vertexBufferHandle = vertexBuffer.getHandle();
    VkDeviceSize offset = 0;
    const auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<VkPipelineStageFlags> flags = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    std::vector<VkSemaphore> waitSemaphores {imageAvailableSemaphores[0].get(), imageAvailableSemaphores[1].get()};
    std::vector<VkSemaphore> signalSemaphores {renderFinishedSemaphores[0].get(), renderFinishedSemaphores[1].get()};

    SDL_Event event;
    uint32_t imageIndex = 0;
    uint32_t currentFrame = 0;

    float angle = 0.5f;

    // Main loop.
    while (true) {
        if (SDL_PollEvent(&event)) {
            if ((event.type == SDL_QUIT) || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                break;
            if ((event.type == SDL_KEYDOWN) && event.key.keysym.sym == SDLK_RIGHT)
                angle += 0.2f;
            else if ((event.type == SDL_KEYDOWN) && event.key.keysym.sym == SDLK_LEFT)
                angle -= 0.2f;
        }

        std::vector<VkFence> fenceToWait{fences[currentFrame].get()};
        _logicalDevice.waitForFences(fenceToWait, true);
        imageIndex = swapChain.acquireNextImage(imageAvailableSemaphores[currentFrame].get());
        _logicalDevice.resetFences(fenceToWait);

        // Update uniform buffer.
        const auto& currentTime = std::chrono::high_resolution_clock::now();
        const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        // Update camera position based on angle
        constexpr float radius = 25.f;
        math::vec3f camera;
        float speed = 0.5f;
        camera.x = radius * cos(angle);
        camera.y = 0.f;
        camera.z = radius * sin(angle);
        uniformBuffers[currentFrame]->fill(&ubo);


        vulkan::CommandBuffer cmdBuf = commandPool.getBuffer(currentFrame);
        cmdBuf.reset();
        cmdBuf.begin();
            cmdBuf.setViewports(viewPorts);
            cmdBuf.setScissors(scissors);
            cmdBuf.bindVertexBuffers(0, 1, &vertexBufferHandle, &offset);
            cmdBuf.bindIndexBuffer(indexBuffer.getHandle(), 0, VK_INDEX_TYPE_UINT32);
            cmdBuf.bindPipeline(graphicsPipeline.get(), VK_PIPELINE_BIND_POINT_GRAPHICS);
            cmdBuf.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBuilder.getPipelineLayout(), 0, 1, &descriptorSetPool.getSet(currentFrame));

            cmdBuf.beginRenderPass(swapChain, renderPassPtr.get(), extent, {0, 0}, imageIndex);
                cmdBuf.drawIndexed(indexBuffer.getSizeBytes() / avocado::vulkan::sizeOf<indexBuffer.getIndexType()>(), 1, 0,0,0);
            cmdBuf.endRenderPass();
        cmdBuf.end();

        auto submitInfo = graphicsQueue.createSubmitInfo(waitSemaphores[currentFrame], signalSemaphores[currentFrame], cmdBuf.getHandle(), flags);
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

