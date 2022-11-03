#include "commandbuffer.hpp"

#include "swapchain.hpp"
#include "vkutils.hpp"

#include <cassert>

using namespace std::string_literals;

namespace avocado::vulkan {

CommandBuffer::CommandBuffer(VkCommandBuffer buf):
    _buf(buf) {
}

VkCommandBuffer CommandBuffer::getHandle() {
    return _buf;
}

bool CommandBuffer::isValid() const {
    return (_buf != VK_NULL_HANDLE);
}

void CommandBuffer::begin() {
    auto beginInfo = createStruct<VkCommandBufferBeginInfo>();
    const VkResult result = vkBeginCommandBuffer(_buf, &beginInfo);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkBeginCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::end() {
    const VkResult result = vkEndCommandBuffer(_buf);
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkEndCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::beginRenderPass(Swapchain &swapchain, VkRenderPass renderPass, const VkExtent2D extent, const VkOffset2D offset, const uint32_t imageIndex) {
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

void CommandBuffer::endRenderPass() {
    assert(_buf != VK_NULL_HANDLE);

    vkCmdEndRenderPass(_buf);
}

void CommandBuffer::draw(const uint32_t vertexCount, const uint32_t instanceCount,
        const uint32_t firstVertex, const uint32_t firstInstance) {
    vkCmdDraw(_buf, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::reset(const CommandBuffer::ResetFlags flags) {
    const VkResult result = vkResetCommandBuffer(_buf, static_cast<uint32_t>(flags));
    setHasError(result != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkResetCommandBuffer returned "s + getVkResultString(result));
    }
}

void CommandBuffer::setViewports(const std::vector<VkViewport> &vps, const uint32_t firstIndex, const uint32_t count) {
    vkCmdSetViewport(_buf, firstIndex, count, vps.data());
}

void CommandBuffer::setScissors(const std::vector<VkRect2D> &scissors, const uint32_t firstIndex, const uint32_t count) {
    vkCmdSetScissor(_buf, firstIndex, count, scissors.data());
}

void CommandBuffer::bindPipeline(VkPipeline pipeline, const CommandBuffer::PipelineBindPoint bindPoint) {
    vkCmdBindPipeline(_buf, static_cast<VkPipelineBindPoint>(bindPoint), pipeline);
}

}

