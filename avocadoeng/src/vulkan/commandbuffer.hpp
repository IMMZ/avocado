#ifndef AVOCADO_VULKAN_COMMANDBUFFER_HPP
#define AVOCADO_VULKAN_COMMANDBUFFER_HPP

#include "../errorstorage.hpp"

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

class Swapchain;

class CommandBuffer: public core::ErrorStorage {
public:
    explicit CommandBuffer(VkCommandBuffer buf);

    VkCommandBuffer getHandle();
    bool isValid() const;

    void begin();
    void end();
    void beginRenderPass(Swapchain &swapchain, VkRenderPass renderPass, const VkExtent2D extent, const VkOffset2D offset);
    void endRenderPass();

    void draw(const uint32_t vertexCount, const uint32_t instanceCount,
        const uint32_t firstVertex, const uint32_t firstInstance);

    enum class ResetFlags: uint32_t {
        NoFlags = 0
        , ReleaseResources = 0x00000001
    };
    void reset(const ResetFlags flags);

    void setViewport(VkViewport vp);
    void setScissor(VkRect2D scissor);

    enum class PipelineBindPoint {
        Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
        Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
        RayTracing = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
        SubpassShadingHuawei = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI,
        RayTracingNV = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV 
    };
    void bindPipeline(VkPipeline pipeline, const PipelineBindPoint bindPoint);

private:
    VkCommandBuffer _buf;
};

}
#endif

