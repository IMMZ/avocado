#include "commandbuffer.hpp"

#include "buffer.hpp"
#include "swapchain.hpp"
#include "vkutils.hpp"

#include <cassert>

using namespace std::string_literals;

namespace avocado::vulkan {

CommandBuffer::CommandBuffer(VkCommandBuffer buf):
    _buf(buf) {
}

VkCommandBuffer CommandBuffer::getHandle() noexcept {
    return _buf;
}

bool CommandBuffer::isValid() const noexcept {
    return (_buf != VK_NULL_HANDLE);
}

void CommandBuffer::begin(const VkCommandBufferUsageFlags flags) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    auto beginInfo = createStruct<VkCommandBufferBeginInfo>();
    beginInfo.flags = flags;
    const VkResult result = vkBeginCommandBuffer(_buf, &beginInfo);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkBeginCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::end() noexcept {
    assert(_buf != VK_NULL_HANDLE);

    const VkResult result = vkEndCommandBuffer(_buf);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkEndCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::beginRenderPass(Swapchain &swapchain, VkRenderPass renderPass, const VkExtent2D extent, const VkOffset2D offset, const uint32_t imageIndex) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    auto renderPassInfo = createStruct<VkRenderPassBeginInfo>();
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchain.getFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // todo extract as parameter?

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(_buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass() noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdEndRenderPass(_buf);
}

void CommandBuffer::copyBuffer(Buffer &srcBuf, Buffer &dstBuf, const std::vector<VkBufferCopy> &regions) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdCopyBuffer(_buf, srcBuf.getHandle(), dstBuf.getHandle(),
        static_cast<uint32_t>(regions.size()), regions.data());
}

void CommandBuffer::bindVertexBuffers(const uint32_t firstBinding, const uint32_t bindingCount,
    VkBuffer *buffers, VkDeviceSize *offsets) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdBindVertexBuffers(_buf, firstBinding, bindingCount, buffers, offsets);
}

void CommandBuffer::bindIndexBuffer(VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdBindIndexBuffer(_buf, buffer, offset, indexType);
}

void CommandBuffer::draw(const uint32_t vertexCount, const uint32_t instanceCount,
        const uint32_t firstVertex, const uint32_t firstInstance) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdDraw(_buf, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(const uint32_t indexCount, const uint32_t instanceCount,
        const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdDrawIndexed(_buf, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void CommandBuffer::reset(const CommandBuffer::ResetFlags flags) {
    assert(_buf != VK_NULL_HANDLE);

    const VkResult result = vkResetCommandBuffer(_buf, static_cast<uint32_t>(flags));
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkResetCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::setViewports(const std::vector<VkViewport> &vps, const uint32_t firstIndex, const uint32_t count) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdSetViewport(_buf, firstIndex, count, vps.data());
}

void CommandBuffer::setScissors(const std::vector<VkRect2D> &scissors, const uint32_t firstIndex, const uint32_t count) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdSetScissor(_buf, firstIndex, count, scissors.data());
}

void CommandBuffer::bindPipeline(VkPipeline pipeline, const CommandBuffer::PipelineBindPoint bindPoint) noexcept {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdBindPipeline(_buf, static_cast<VkPipelineBindPoint>(bindPoint), pipeline);
}

}

