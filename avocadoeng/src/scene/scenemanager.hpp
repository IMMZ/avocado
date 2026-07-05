#ifndef SCENE_SCENEMANAGER_HPP
#define SCENE_SCENEMANAGER_HPP

#include "camera.hpp"
#include "scene.hpp"

#include <vector>

#include <../../third_party/tinygltf-release/tiny_gltf.h>

namespace avocado::core {

enum class ModelTopology {
    Points
    , Lines
    , LineLoop
    , LineStrip
    , Triangles
    , TriangleStrip
    , TriangleFan
};


enum class AccessorType: int {
    Scalar = TINYGLTF_TYPE_SCALAR
    , Vec2 = TINYGLTF_TYPE_VEC2
    , Vec3 = TINYGLTF_TYPE_VEC3
    , Vec4 = TINYGLTF_TYPE_VEC4
    , Mat2 = TINYGLTF_TYPE_MAT2
    , Mat3 = TINYGLTF_TYPE_MAT3
    , Mat4 = TINYGLTF_TYPE_MAT4
};

[[nodiscard]] constexpr size_t getNumberOfComponents(const AccessorType accessorType) noexcept {
    if (accessorType == AccessorType::Scalar) return 1;
    if (accessorType == AccessorType::Vec2) return 2;
    if (accessorType == AccessorType::Vec3) return 3;
    if (accessorType == AccessorType::Vec4) return 4;
    if (accessorType == AccessorType::Mat2) return 4;
    if (accessorType == AccessorType::Mat3) return 9;
    if (accessorType == AccessorType::Mat4) return 16;

    return 0;
}

enum class ComponentType: int {
    Byte = TINYGLTF_COMPONENT_TYPE_BYTE
    , UByte = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE
    , Short = TINYGLTF_COMPONENT_TYPE_SHORT
    , UShort = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
    , Int = TINYGLTF_COMPONENT_TYPE_INT
    , UInt = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
    , Float = TINYGLTF_COMPONENT_TYPE_FLOAT
    , Double = TINYGLTF_COMPONENT_TYPE_DOUBLE
};

[[nodiscard]] constexpr size_t getComponentTypeSize(const ComponentType componentType) noexcept {
    if (componentType == ComponentType::Byte) return 1;
    if (componentType == ComponentType::UByte) return 1;
    if (componentType == ComponentType::Short) return 2;
    if (componentType == ComponentType::UShort) return 2;
    if (componentType == ComponentType::Int) return 4;
    if (componentType == ComponentType::UInt) return 4;
    if (componentType == ComponentType::Float) return 4;
    if (componentType == ComponentType::Double) return 8;

    return 0;
}

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
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

struct SceneManager {
    std::vector<Node> nodes;
    std::vector<Scene> _scenes;
    std::vector<Sampler> _samplers;
    std::vector<Image> _images;
    std::vector<Texture> _textures;
    std::vector<Camera> _cameras;
    Scene * _defaultScene = nullptr;

    void load(const std::string &filepath);

    std::vector<Vertex> copyVerticesFromPrimitive(const tinygltf::Primitive &primitive);
    std::vector<uint32_t> copyIndiciesFromPrimitive(const tinygltf::Primitive &primitive);
    tinygltf::Primitive* findPrimitive(const int32_t primitiveIndex);
    avocado::vulkan::Buffer formVertexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device, const int32_t primitiveIndex);
    avocado::vulkan::Buffer formIndexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device, const int32_t primitiveIndex);
    int32_t getMaterialIndex(const int32_t primitiveIndex);
    VkPrimitiveTopology getPrimitiveTopology(const int32_t primitiveIndex) noexcept;
    VkCullModeFlags getCullMode(const int32_t primitiveIndex) noexcept;

    [[nodiscard]] inline int32_t getPrimitivesCount() const noexcept {
        return _primitivesCount;
    }

    [[nodiscard]] bool hasCameras() const noexcept {
        return !_cameras.empty();
    }

    Camera& getCamera() {
        return _cameras[0];
    }

private:
    void parseNodes();
    void parseScenes();
    void parseCameras();
    void parseSamplers();
    void parseImages();
    void parseTextures();

    tinygltf::Model _model;
    int32_t _primitivesCount = 0;
    bool _nodesAreParsed = false;
};

} // namespace avocado::core

#endif // ifndef SCENE_SCENEMANAGER_HPP
