#ifndef AVOCADO_VULKAN_COMMANDBUFFER_HPP
#define AVOCADO_VULKAN_COMMANDBUFFER_HPP

#include "../errorstorage.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class Buffer;
class Image;
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
    void copyBufferToImage(Buffer &buffer, Image &image, const uint32_t width, const uint32_t height);
    void bindVertexBuffers(const uint32_t firstBinding, const uint32_t bindingCount, VkBuffer *buffers, VkDeviceSize *offsets) noexcept;
    void bindIndexBuffer(VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) noexcept;
    void bindDescriptorSets(const VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, uint32_t firstSet, uint32_t setCount, const VkDescriptorSet *sets, uint32_t dynamicOffsetCount = 0, const uint32_t *dynamicOffsets = nullptr);

    void draw(const uint32_t vertexCount, const uint32_t instanceCount,
        const uint32_t firstVertex = 0, const uint32_t firstInstance = 0) noexcept;
    void drawIndexed(const uint32_t indexCount, const uint32_t instanceCount,
        const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) noexcept;

    void reset(const VkCommandPoolResetFlagBits flags);
    void setViewports(const std::vector<VkViewport> &vps, const uint32_t firstIndex, const uint32_t count) noexcept;
    inline void setViewports(const std::vector<VkViewport> &vps) {
        setViewports(vps, 0, static_cast<uint32_t>(vps.size()));
    }

    void setScissors(const std::vector<VkRect2D> &scissors, const uint32_t firstIndex, const uint32_t count) noexcept;
    inline void setScissors(const std::vector<VkRect2D> &scissors) {
        setScissors(scissors, 0, static_cast<uint32_t>(scissors.size()));
    }

    void bindPipeline(VkPipeline pipeline, const VkPipelineBindPoint bindPoint) noexcept;

private:
    VkCommandBuffer _buf = VK_NULL_HANDLE;
};

}
#endif

