#ifndef SCENETEXTURES_HPP
#define SCENETEXTURES_HPP

#include "buffer.hpp"
#include "image.hpp"
#include "pointertypes.hpp"

#include <vector>

namespace avocado::core {

class Image;
class SceneManager;
class Texture;

}

namespace avocado::vulkan {

class DescriptorSetPool;
class LogicalDevice;
class PhysicalDevice;
class QueueManager;
class Swapchain;

class SceneTextures {
public:
    explicit SceneTextures(PhysicalDevice &physicalDevice, LogicalDevice &logicalDevice);
    void load(const core::SceneManager &sceneManager, Swapchain &swapchain, QueueManager &queueManager);
    [[nodiscard]] inline size_t getSamplersCount() const noexcept {
        return _samplers.size();
    }

    DescriptorSetPool exportToDescriptorSet();

private:
    void loadImage(const core::Texture &texture, Swapchain &swapchain);
    void loadSampler(const core::Texture &texture);

    std::vector<size_t> _samplersMapping;
    std::vector<size_t> _imagesMapping;
    std::vector<Image> _images;
    std::vector<Buffer> _buffers;
    std::vector<ImageViewPtr> _imageViews;
    std::vector<SamplerPtr> _samplers;
    PhysicalDevice &_physicalDevice;
    LogicalDevice &_logicalDevice;
};

} // namespace avocado::vulkan

#endif // SCENETEXTURES_HPP

