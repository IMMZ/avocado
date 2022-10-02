#include "commandbuffer.hpp"

#include "swapchain.hpp"

#include <cassert>

#include <iostream> // todo remove

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
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (VK_SUCCESS != vkBeginCommandBuffer(_buf, &beginInfo)) {

        setHasError(true);
        // todo fix
        // setErrorMessage("")
    }
}

void CommandBuffer::end() {
    if (vkEndCommandBuffer(_buf) != VK_SUCCESS) {
        setHasError(true);
        // todo fix
        // setErrorMessage
    }
}

void CommandBuffer::beginRenderPass(Swapchain &swapchain, VkRenderPass renderPass, const VkExtent2D extent, const VkOffset2D offset) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchain.getFramebuffer(0); // todo what's the 0? Change this hack.
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
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
        ; // todo fix
          // setErrorMessage()
    }
}

// todo make setViewportS (many ones). The same is for scissor.
void CommandBuffer::setViewport(VkViewport vp) {
    std::cout << "VIEW PORT IS SET" << std::endl;
    vkCmdSetViewport(_buf, 0, 1, &vp);
}

void CommandBuffer::setScissor(VkRect2D scissor) {
    vkCmdSetScissor(_buf, 0, 1, &scissor);
}



void CommandBuffer::bindPipeline(VkPipeline pipeline, const CommandBuffer::PipelineBindPoint bindPoint) {
    vkCmdBindPipeline(_buf, static_cast<VkPipelineBindPoint>(bindPoint), pipeline);
}

}

