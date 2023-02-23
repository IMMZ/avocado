#ifndef AVOCADO_VULKAN_COMMANDBUFFER_HPP
#define AVOCADO_VULKAN_COMMANDBUFFER_HPP

#include "../errorstorage.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class Buffer;
class Swapchain;

class CommandBuffer: public core::ErrorStorage {
public:
    CommandBuffer() = default;
    explicit CommandBuffer(VkCommandBuffer buf);

    VkCommandBuffer getHandle() noexcept;
    bool isValid() const noexcept;

    void begin(const VkCommandBufferUsageFlags flags = 0) noexcept;
    void end() noexcept;
    void beginRenderPass(Swapchain &swapchain, VkRenderPass renderPass, const VkExtent2D extent, const VkOffset2D offset, const uint32_t imageIndex) noexcept;
    void endRenderPass() noexcept;

    void copyBuffer(Buffer &srcBuf, Buffer &dstBuf, const std::vector<VkBufferCopy> &regions) noexcept;

    void draw(const uint32_t vertexCount, const uint32_t instanceCount,
        const uint32_t firstVertex = 0, const uint32_t firstInstance = 0) noexcept;

    enum class ResetFlags: uint32_t {
        NoFlags = 0
        , ReleaseResources = 0x00000001
    };
    void reset(const ResetFlags flags);

    void setViewports(const std::vector<VkViewport> &vps, const uint32_t firstIndex, const uint32_t count) noexcept;
    inline void setViewports(const std::vector<VkViewport> &vps) {
        setViewports(vps, 0, static_cast<uint32_t>(vps.size()));
    }

    void setScissors(const std::vector<VkRect2D> &scissors, const uint32_t firstIndex, const uint32_t count) noexcept;
    inline void setScissors(const std::vector<VkRect2D> &scissors) {
        setScissors(scissors, 0, static_cast<uint32_t>(scissors.size()));
    }

    enum class PipelineBindPoint {
        Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
        Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
        RayTracing = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
        SubpassShadingHuawei = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI,
        RayTracingNV = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
    };
    void bindPipeline(VkPipeline pipeline, const PipelineBindPoint bindPoint) noexcept;

private:
    VkCommandBuffer _buf = VK_NULL_HANDLE;
};

}
#endif

