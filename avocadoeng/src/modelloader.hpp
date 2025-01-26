#ifndef AVOCADO_CORE_MODELLOADER
#define AVOCADO_CORE_MODELLOADER

#include "vulkan/image.hpp"
#include "vulkan/logicaldevice.hpp"
#include "vulkan/physicaldevice.hpp"
#include "vulkan/pointertypes.hpp"

#include <../../third_party/tinygltf-release/tiny_gltf.h>

#include <cstdint>
#include <string>
#include <utility>

namespace avocado {
class Vertex;

namespace vulkan {
class CommandPool;
class Image;
class Queue;
class Swapchain;
}
}

namespace avocado::core {

class ModelLoader {
public:
    ModelLoader(avocado::vulkan::PhysicalDevice &physicalDevice, avocado::vulkan::LogicalDevice &logicalDevice,
        avocado::vulkan::Swapchain &swapchain, avocado::vulkan::CommandPool &commandPool, avocado::vulkan::Queue &graphicsQueue);

    const uint32_t loadModel(const std::string &filepath);
    // todo Implement
    //void unloadModel(const uint32_t index);

    std::pair<avocado::Vertex*, size_t /* count */> getVertices(const uint32_t index);
    std::pair<uint32_t*, size_t /* count */> getIndices(const uint32_t index);
    avocado::vulkan::Image& getImage(const uint32_t index);
    avocado::vulkan::ImageViewPtr& getImageView(const uint32_t index);
    avocado::vulkan::SamplerPtr& getSampler(const uint32_t index);

    [[nodiscard]] bool hasSamplers() const {
        return !_samplers.empty();
    }

private:
    std::tuple<avocado::vulkan::Image, avocado::vulkan::ImageViewPtr, avocado::vulkan::SamplerPtr> loadTexture(avocado::vulkan::Swapchain &swapChain,
        avocado::vulkan::CommandPool &commandPool, avocado::vulkan::Queue &graphicsQueue, const tinygltf::Image &image);
    std::vector<avocado::Vertex*> _vertices;
    std::vector<size_t> _verticesCounts;
    std::vector<uint32_t*> _indices;
    std::vector<size_t> _indicesCounts;
    std::vector<avocado::vulkan::Image> _images;
    std::vector<avocado::vulkan::ImageViewPtr> _imageViews;
    std::vector<avocado::vulkan::SamplerPtr> _samplers;
    tinygltf::Model _model;

    avocado::vulkan::PhysicalDevice &_physicalDevice;
    avocado::vulkan::LogicalDevice &_logicalDevice;
    avocado::vulkan::CommandPool &_commandPool;
    avocado::vulkan::Queue &_graphicsQueue;
    avocado::vulkan::Swapchain &_swapchain;
};

}

#endif
