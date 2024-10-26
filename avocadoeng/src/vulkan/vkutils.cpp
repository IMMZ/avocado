#include "vkutils.hpp"

#include "../bufferindexconsts.hpp"
#include "commandbuffer.hpp"
#include "commandpool.hpp"
#include "image.hpp"
#include "queue.hpp"

#include <stdexcept>

namespace avocado::vulkan {

void copyBufferToImage(CommandPool &commandPool, vulkan::Queue &queue, vulkan::Buffer &buffer,
    vulkan::Image &image, uint32_t width, uint32_t height) {
    vulkan::CommandBuffer imageCopyCmdBuffer = commandPool.getBuffer(static_cast<size_t>(core::BufferIndex::CopyImage));
    imageCopyCmdBuffer.beginOneTimeSubmit();
        imageCopyCmdBuffer.copyBufferToImage(buffer, image, width, height);
    imageCopyCmdBuffer.endOneTimeAndSubmit(queue);
}

void transitImageLayout(CommandBuffer &cmdBuf, Queue &queue, Image &image, VkFormat format, VkImageLayout oldLayout,
    VkImageLayout newLayout, const VkImageAspectFlags aspectFlags) {
    VkImageMemoryBarrier barrier{}; FILL_S_TYPE(barrier);
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image.getHandle();
    barrier.subresourceRange.aspectMask = aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM, dstStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
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
            throw std::invalid_argument("unsupported layout transition!"); // todo No exceptions!
    }

    cmdBuf.beginOneTimeSubmit();
        cmdBuf.pipelineBarrier(srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    cmdBuf.endOneTimeAndSubmit(queue);
}

}

