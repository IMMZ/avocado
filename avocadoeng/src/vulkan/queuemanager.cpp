#include "queuemanager.hpp"

#include "buffer.hpp"
#include "image.hpp"
#include "logicaldevice.hpp"
#include "structuretypes.hpp"

namespace {

enum class BufferIndex: size_t {
    Copy
    , Transit
};

}

namespace avocado::vulkan {

QueueManager::QueueManager(LogicalDevice &logicalDevice, const QueueFamily graphicsQueueFamily):
    _commandPool(CommandPool(logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamily)),
    _graphicsQueue(logicalDevice.getGraphicsQueue(graphicsQueueFamily)),
    _logicalDevice(logicalDevice) {
    _commandPool.allocateBuffers(2);
}

void QueueManager::copyBuffersToImages(std::vector<Buffer> &buffers, std::vector<Image> &images) {
    assert(buffers.size() == images.size() && "Images and buffers count must be equal.");

    CommandBuffer copyBuffer = _commandPool.getBuffer(avocado::utils::enumToInteger(BufferIndex::Copy));
    copyBuffer.begin(); // todo Decide later if we could use here one time submit strategy.
    for (size_t i = 0; i < buffers.size(); ++i)
        copyBuffer.copyBufferToImage(buffers[i], images[i]);
    copyBuffer.endOneTimeAndSubmit(_graphicsQueue);
}

void QueueManager::imageTransitionBarriers(std::vector<Image> &images, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags) {
    DEFINE_VK_STRUCTURE(VkImageMemoryBarrier, barrier);
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_NONE, dstStage = VK_PIPELINE_STAGE_NONE;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        assert(false && "Unsupported layout transition!");
    }

    std::vector<VkImageMemoryBarrier> barriers(images.size(), barrier);
    for (size_t i = 0; i < barriers.size(); ++i)
        barriers[i].image = images[i].getHandle();

    CommandBuffer transitBuffer = _commandPool.getBuffer(avocado::utils::enumToInteger(BufferIndex::Transit));
    transitBuffer.reset();
    transitBuffer.begin(); // todo Decide later if we could use here one time submit strategy.
        transitBuffer.pipelineBarrier(srcStage, dstStage, 0, 0, nullptr, 0, nullptr,
            barriers.size(), barriers.data());
    transitBuffer.end();


    DEFINE_VK_STRUCTURE(VkSubmitInfo, submitInfo);
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer handle = transitBuffer.getHandle();
    submitInfo.pCommandBuffers = &handle;
    _graphicsQueue.submit(submitInfo);
    _graphicsQueue.waitIdle();
}

} // namespace avocado::vulkan

