#ifndef AVOCADO_VULKAN_LOGICAL_DEVICE
#define AVOCADO_VULKAN_LOGICAL_DEVICE

#include "commandbuffer.hpp"

#include "../errorstorage.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <vector>

namespace avocado::vulkan {

class LogicalDevice: public avocado::core::ErrorStorage {
public:
    LogicalDevice();
    explicit LogicalDevice(VkDevice dev);

    VkDevice getHandle();
    bool isValid() const;

    enum class ShaderType {
        Vertex = VK_SHADER_STAGE_VERTEX_BIT,
        TessellationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        TessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
        Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        Compute = VK_SHADER_STAGE_COMPUTE_BIT,
        AllGraphics = VK_SHADER_STAGE_ALL_GRAPHICS,
        All = VK_SHADER_STAGE_ALL,
        RaygenKHR = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        AnyHitKHR = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        ClosestHitKHR = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        MissKHR = VK_SHADER_STAGE_MISS_BIT_KHR,
        IntersectionKHR = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        CallableKHR = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        TaskNV = VK_SHADER_STAGE_TASK_BIT_NV,
        MeshNV = VK_SHADER_STAGE_MESH_BIT_NV,
        SubpassShadingHuawei = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI,
        RaygenNV = VK_SHADER_STAGE_RAYGEN_BIT_NV,
        AnyHitNV = VK_SHADER_STAGE_ANY_HIT_BIT_NV,
        ClosestHitNV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
        MissNV = VK_SHADER_STAGE_MISS_BIT_NV,
        IntersectionNV = VK_SHADER_STAGE_INTERSECTION_BIT_NV,
        CallableNV = VK_SHADER_STAGE_CALLABLE_BIT_NV
    };

    VkPipelineShaderStageCreateInfo addShaderModule(const std::vector<char> &shaderCode, ShaderType shType);
    VkPipelineShaderStageCreateInfo addShaderModule(std::vector<char> &&shaderCode, ShaderType shType);
    VkQueue getQueue(const uint32_t queueFamilyIndex, const uint32_t queueIndex);
    VkFence createFence();
    void waitForFences(const std::vector<VkFence> &fences, const bool waitAll, uint64_t timeout);
    void resetFences(const std::vector<VkFence> &fences);
    VkSemaphore createSemaphore();


    enum class CommandPoolCreationFlags {
        Transient = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        Reset = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        Protected = VK_COMMAND_POOL_CREATE_PROTECTED_BIT,
    };
    VkCommandPool createCommandPool(const CommandPoolCreationFlags flags, const uint32_t queueFamilyIndex);

    enum class CommandBufferLevel {
        Primary = VK_COMMAND_BUFFER_LEVEL_PRIMARY
        , Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY
    };
    std::vector<CommandBuffer> allocateCommandBuffers(const uint32_t count, VkCommandPool cmdPool, const CommandBufferLevel bufLevel);

    void waitIdle();

private:
    VkPipelineShaderStageCreateInfo createShaderModule(ShaderType shType);

    using DeviceDeleter = decltype(std::bind(vkDestroyDevice, std::placeholders::_1, nullptr));
    using DevicePtr = std::unique_ptr<VkDevice_T, DeviceDeleter>; 
    DevicePtr _dev;

    using ShaderModuleDeleter = decltype(std::bind(vkDestroyShaderModule, _dev.get(), std::placeholders::_1, nullptr));
    std::vector<std::unique_ptr<VkShaderModule_T, ShaderModuleDeleter>> _shaderModules;
    std::vector<std::vector<char>> _shaderCodes;

public:
    using RenderPassUniquePtr = std::unique_ptr<
        std::remove_pointer_t<VkRenderPass>
        , decltype(std::bind(vkDestroyRenderPass, _dev.get(), std::placeholders::_1, nullptr))>;
    RenderPassUniquePtr createRenderPass(VkFormat format);
};

} // namespace vulkan.

#endif

