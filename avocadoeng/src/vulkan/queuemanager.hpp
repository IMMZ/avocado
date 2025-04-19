#ifndef QUEUEMANAGER_HPP
#define QUEUEMANAGER_HPP

#include "commandpool.hpp"
#include "queue.hpp"

namespace avocado::vulkan {

class LogicalDevice;

class QueueManager {
public:
    explicit QueueManager(LogicalDevice &logicalDevice, const QueueFamily graphicsQueueFamily);

    /** @brief Copies buffers to images using one time submit command buffer.
     * @param buffers Buffers to copy.
     * @param images Images to copy.
     */
    void copyBuffersToImages(std::vector<Buffer> &buffers, std::vector<Image> &images);
    void imageTransitionBarriers(std::vector<Image> &images, const VkImageLayout oldLayout, const VkImageLayout newLayout,
        const VkImageAspectFlags aspectFlags);

private:
    CommandPool _commandPool;
    Queue _graphicsQueue;
    LogicalDevice &_logicalDevice;
};

} // namespace avocado::vulkan

#endif // QUEUEMANAGER_HPP

