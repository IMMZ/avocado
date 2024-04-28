#include "commandbuffer.hpp"

#include "buffer.hpp"
#include "image.hpp"
#include "structuretypes.hpp"
#include "swapchain.hpp"
#include "vkutils.hpp"

#include <array>
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
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    VkCommandBufferBeginInfo beginInfo{}; FILL_S_TYPE(beginInfo);
    beginInfo.flags = flags;
    const VkResult result = vkBeginCommandBuffer(_buf, &beginInfo);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkBeginCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::end() noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    const VkResult result = vkEndCommandBuffer(_buf);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkEndCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::beginRenderPass(Swapchain &swapchain, VkRenderPass renderPass, const VkExtent2D extent, const VkOffset2D offset, const uint32_t imageIndex) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    VkRenderPassBeginInfo renderPassInfo{}; FILL_S_TYPE(renderPassInfo);
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchain.getFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 2> clearColors{};
    clearColors[0].color = {{0.f, 0.f, 0.f, 0.f}};
    clearColors[1].depthStencil = {1.f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();
    vkCmdBeginRenderPass(_buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass() noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdEndRenderPass(_buf);
}

void CommandBuffer::copyBuffer(Buffer &srcBuf, Buffer &dstBuf, const std::vector<VkBufferCopy> &regions) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdCopyBuffer(_buf, srcBuf.getHandle(), dstBuf.getHandle(),
        static_cast<uint32_t>(regions.size()), regions.data());
}

void CommandBuffer::copyBufferToImage(Buffer &buffer, Image &image, const uint32_t width, const uint32_t height) {
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

    vkCmdCopyBufferToImage(_buf, buffer.getHandle(), image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void CommandBuffer::bindVertexBuffers(const uint32_t firstBinding, const uint32_t bindingCount,
    VkBuffer *buffers, VkDeviceSize *offsets) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdBindVertexBuffers(_buf, firstBinding, bindingCount, buffers, offsets);
}

void CommandBuffer::bindIndexBuffer(VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdBindIndexBuffer(_buf, buffer, offset, indexType);
}

void CommandBuffer::bindDescriptorSets(const VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout,
    uint32_t firstSet,
    uint32_t setCount, const VkDescriptorSet *sets,
    uint32_t dynamicOffsetCount, const uint32_t *dynamicOffsets) {
    vkCmdBindDescriptorSets(_buf, bindPoint, pipelineLayout, firstSet, setCount, sets, dynamicOffsetCount, dynamicOffsets);
}

void CommandBuffer::draw(const uint32_t vertexCount, const uint32_t instanceCount,
        const uint32_t firstVertex, const uint32_t firstInstance) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdDraw(_buf, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(const uint32_t indexCount, const uint32_t instanceCount,
        const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdDrawIndexed(_buf, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::reset(const VkCommandPoolResetFlagBits flags) {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    const VkResult result = vkResetCommandBuffer(_buf, flags);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkResetCommandBuffer returned "s + getVkResultString(result));
}

void CommandBuffer::setViewports(const std::vector<VkViewport> &vps, const uint32_t firstIndex, const uint32_t count) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdSetViewport(_buf, firstIndex, count, vps.data());
}

void CommandBuffer::setScissors(const std::vector<VkRect2D> &scissors, const uint32_t firstIndex, const uint32_t count) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");
    vkCmdSetScissor(_buf, firstIndex, count, scissors.data());
}

void CommandBuffer::bindPipeline(VkPipeline pipeline, const VkPipelineBindPoint bindPoint) noexcept {
    assert(_buf != VK_NULL_HANDLE && "Handle mustn't be null.");

    vkCmdBindPipeline(_buf, bindPoint, pipeline);
}

}

