#include "commandbuffer.hpp"

#include "buffer.hpp"
#include "image.hpp"
#include "queue.hpp"
#include "structuretypes.hpp"
#include "swapchain.hpp"

#include <array>

using namespace std::literals::string_literals;

namespace avocado::vulkan {

CommandBuffer::CommandBuffer(const VkCommandBuffer cmdBuf):
    _cmdBuf(cmdBuf) {
}

const VkCommandBuffer& CommandBuffer::getHandle() const noexcept {
    return _cmdBuf;
}

VkCommandBuffer& CommandBuffer::getHandle() noexcept {
    return _cmdBuf;
}

void CommandBuffer::begin(const VkCommandBufferUsageFlags flags) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");

    VkCommandBufferBeginInfo beginInfo{}; FILL_S_TYPE(beginInfo);
    beginInfo.flags = flags;
    const VkResult result = vkBeginCommandBuffer(_cmdBuf, &beginInfo);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkBeginCommandBuffer returned "s + getVkResultString(result));
}

void CommandBuffer::end() noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");

    const VkResult result = vkEndCommandBuffer(_cmdBuf);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkEndCommandBuffer returned "s + getVkResultString(result));
}

void CommandBuffer::beginOneTimeSubmit() {
    begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void CommandBuffer::endOneTimeAndSubmit(Queue &queue) {
    end();

    VkSubmitInfo submitInfo{}; FILL_S_TYPE(submitInfo);
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_cmdBuf;
    queue.submit(submitInfo);
    setHasError(queue.hasError());
    if (hasError()) {
        setErrorMessage(queue.getErrorMessage());
        return;
    }

    queue.waitIdle();
    setHasError(queue.hasError());
    if (hasError())
        setErrorMessage(queue.getErrorMessage());
}

void CommandBuffer::beginRenderPass(Swapchain &swapchain, VkRenderPass renderPass, const VkExtent2D extent, const VkOffset2D offset, const uint32_t imageIndex) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");

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
    vkCmdBeginRenderPass(_cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass() noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdEndRenderPass(_cmdBuf);
}

void CommandBuffer::copyBuffer(Buffer &srcBuf, Buffer &dstBuf, const std::vector<VkBufferCopy> &regions) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdCopyBuffer(_cmdBuf, srcBuf.getHandle(), dstBuf.getHandle(), static_cast<uint32_t>(regions.size()), regions.data());
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

    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdCopyBufferToImage(_cmdBuf, buffer.getHandle(), image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void CommandBuffer::bindVertexBuffers(const uint32_t firstBinding, const uint32_t bindingCount,
    VkBuffer *buffers, VkDeviceSize *offsets) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdBindVertexBuffers(_cmdBuf, firstBinding, bindingCount, buffers, offsets);
}

void CommandBuffer::bindIndexBuffer(VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdBindIndexBuffer(_cmdBuf, buffer, offset, indexType);
}

void CommandBuffer::bindDescriptorSets(const VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout,
    uint32_t firstSet,
    uint32_t setCount, const VkDescriptorSet *sets,
    uint32_t dynamicOffsetCount, const uint32_t *dynamicOffsets) {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdBindDescriptorSets(_cmdBuf, bindPoint, pipelineLayout, firstSet, setCount, sets, dynamicOffsetCount, dynamicOffsets);
}

void CommandBuffer::pipelineBarrier(const VkPipelineStageFlags srcStage, const VkPipelineStageFlags dstStage,
    const VkDependencyFlags dependencyFlags, const uint32_t memoryBarriersCount, VkMemoryBarrier *memoryBarriers,
    const uint32_t bufferMemoryBarriersCount, const VkBufferMemoryBarrier *bufBarriers,
    const uint32_t imageMemoryBarriersCount, const VkImageMemoryBarrier *imageBarriers) {
    vkCmdPipelineBarrier(_cmdBuf, srcStage, dstStage, dependencyFlags, memoryBarriersCount, memoryBarriers, bufferMemoryBarriersCount, bufBarriers,
        imageMemoryBarriersCount, imageBarriers);
}

void CommandBuffer::draw(const uint32_t vertexCount, const uint32_t instanceCount,
        const uint32_t firstVertex, const uint32_t firstInstance) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdDraw(_cmdBuf, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(const uint32_t indexCount, const uint32_t instanceCount,
        const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdDrawIndexed(_cmdBuf, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::reset(const VkCommandPoolResetFlagBits flags) {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");

    const VkResult result = vkResetCommandBuffer(_cmdBuf, flags);
    setHasError(result != VK_SUCCESS);
    if (hasError())
        setErrorMessage("vkResetCommandBuffer returned "s + getVkResultString(result));
}

void CommandBuffer::setViewports(const std::vector<VkViewport> &vps, const uint32_t firstIndex, const uint32_t count) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdSetViewport(_cmdBuf, firstIndex, count, vps.data());
}

void CommandBuffer::setScissors(const std::vector<VkRect2D> &scissors, const uint32_t firstIndex, const uint32_t count) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdSetScissor(_cmdBuf, firstIndex, count, scissors.data());
}

void CommandBuffer::bindPipeline(VkPipeline pipeline, const VkPipelineBindPoint bindPoint) noexcept {
    assert(_cmdBuf != VK_NULL_HANDLE && "Null command buffer.");
    vkCmdBindPipeline(_cmdBuf, bindPoint, pipeline);
}

} // namespace avocado::vulkan.

