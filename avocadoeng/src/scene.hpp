#ifndef SCENE_HPP
#define SCENE_HPP

#include "vertex.hpp"

#include "math/matrix.hpp"
#include "math/quaternion.hpp"

#include "vulkan/buffer.hpp"
#include "vulkan/image.hpp"
#include "vulkan/vulkan_core.h"

#include <../../third_party/tinygltf-release/tiny_gltf.h>

namespace avocado::core {

struct Material {
    int foo = 4; // todo remove
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct Node {
    std::vector<Node*> _children;
    std::string _name;
    Mesh *mesh = nullptr;
    avocado::math::Mat4x4 *matrix = nullptr;
    avocado::math::Quaternion *rotation = nullptr;
    avocado::math::vec3f * translation = nullptr;

    [[nodiscard]] inline bool hasTranslation() noexcept {
        return (nullptr != nullptr);
    }

    void addChild(Node * const childNode);
    [[nodiscard]] bool hasMatrix() const noexcept;
    [[nodiscard]] bool hasMesh() const noexcept;
    [[nodiscard]] bool hasRotation() const noexcept;
};

struct Scene {
    std::vector<Node*> _rootNodes;
    std::string _name;
};


#define TINYGLTF_TEXTURE_WRAP_REPEAT (10497)
#define TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE (33071)
#define TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT (33648)

struct Sampler {
    enum class MinFilter {
        None = -1
        , Nearest = VK_FILTER_NEAREST
        , Linear = VK_FILTER_LINEAR

        // todo Is below mapping valid? No similar values in VK_FILTER for GLTF?
        , NearestMipMapNearest = VK_FILTER_NEAREST
        , LinearMipMapNearest = VK_FILTER_LINEAR
        , NearestMipMapLinear = VK_FILTER_NEAREST
        , LinearMipMapLinear = VK_FILTER_LINEAR
    };
    MinFilter _minFilter = MinFilter::None;

    enum class MagFilter {
        None = VK_FILTER_MAX_ENUM
        , Linear = VK_FILTER_LINEAR
        , Nearest = VK_FILTER_NEAREST
    };
    MagFilter _magFilter = MagFilter::None;

    enum class TextureWrap {
        Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT
        , MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
        , ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    };
    TextureWrap _wrapS = TextureWrap::Repeat;
    TextureWrap _wrapT = TextureWrap::Repeat;

    VkSamplerCreateInfo generateVulkanCreateInfo() const;
};

struct Image {
    std::vector<unsigned char> _data;
    std::string _uri;
    int _width = -1;
    int _height = -1;

    enum class MimeType { Png, Jpg, Bmp, Gif };
    MimeType _mimeType = MimeType::Png;
};

struct Texture {
    Image *_image = nullptr;
    Sampler *_sampler = nullptr;
};

// General database.
struct SceneManager {
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Scene> _scenes;
    std::vector<Sampler> _samplers;
    std::vector<Image> _images;
    std::vector<Texture> _textures;
    Scene * _defaultScene = nullptr;

    void load(const std::string &filepath);
    avocado::vulkan::Buffer formVertexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device);
    avocado::vulkan::Buffer formIndexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device);


private:
    void parseNodes();
    void parseScenes();
    void parseSamplers();
    void parseImages();
    void parseTextures();

    std::vector<avocado::vulkan::Image> _vulkanImages; 
    std::vector<avocado::vulkan::ImageViewPtr> _vulkanImageViews;
    std::vector<avocado::vulkan::SamplerPtr> _vulkanSamplers;

    tinygltf::Model model;
    bool _nodesAreParsed = false;
};

}

#endif /* ifndef SCENE_HPP */
