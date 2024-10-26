
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define JSON_NOEXCEPTION

#include "modelloader.hpp"

#include "bufferindexconsts.hpp"
#include "vertex.hpp"
#include "utils.hpp"

#include "vulkan/buffer.hpp"
#include "vulkan/commandbuffer.hpp"
#include "vulkan/commandpool.hpp"
#include "vulkan/swapchain.hpp"
#include "vulkan/vkutils.hpp"

#include <functional>
#include <memory>

namespace {

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

constexpr size_t getNumberOfComponents(const AccessorType accessorType) {
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

constexpr size_t getComponentTypeSize(const ComponentType componentType) {
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

}

namespace avocado::core {

ModelLoader::ModelLoader(avocado::vulkan::PhysicalDevice &physicalDevice, avocado::vulkan::LogicalDevice &logicalDevice,
    avocado::vulkan::Swapchain &swapchain, avocado::vulkan::CommandPool &commandPool, avocado::vulkan::Queue &graphicsQueue):
    _physicalDevice(physicalDevice),
    _logicalDevice(logicalDevice),
    _commandPool(commandPool),
    _graphicsQueue(graphicsQueue),
    _swapchain(swapchain)
    {}

const uint32_t ModelLoader::loadModel(const std::string &filepath) {
    auto loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadASCIIFromFile);
    if (utils::hasExtension(filepath, ".glb"))
        loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadBinaryFromFile);

    tinygltf::TinyGLTF loader;
    std::string error, warning;
    bool loadOk = false;
    loadOk = loadFunction(loader, &_model, &error, &warning, filepath, tinygltf::REQUIRE_VERSION);
    if (!loadOk) {
        return {};
    }

    std::unique_ptr<float[]> positions = nullptr; size_t positionsCount = 0;
    std::unique_ptr<float[]> texCoords = nullptr; size_t texCoordsCount = 0;
    uint32_t *indices = nullptr; size_t indicesCount = 0;
    float *positionData = nullptr;
    unsigned char *indicesData = nullptr;
    float *texCoordData = nullptr;
    Vertex *vertices = nullptr;
    tinygltf::Image *imageToLoad = nullptr;
    for (const tinygltf::Mesh &mesh: _model.meshes) {
         for (const tinygltf::Primitive &primitive: mesh.primitives) {
            const ModelTopology topology = static_cast<ModelTopology>(primitive.mode);
            const auto &attributes = primitive.attributes;
            const size_t accessorIndex = attributes.at("POSITION");
            positionsCount = _model.accessors[accessorIndex].count;
            const size_t accessorOffset = _model.accessors[accessorIndex].byteOffset;
            const size_t bufferViewIndex = _model.accessors[accessorIndex].bufferView;
            const size_t bufferIndex = _model.bufferViews[bufferViewIndex].buffer;
            const size_t length = _model.bufferViews[bufferViewIndex].byteLength;
            const size_t offset = _model.bufferViews[bufferViewIndex].byteOffset;
            const size_t bytesStride = _model.bufferViews[bufferViewIndex].byteStride;

            const AccessorType accessorType = static_cast<AccessorType>(_model.accessors[accessorIndex].type);
            const size_t numberOfComponents = getNumberOfComponents(accessorType);

            positions.reset(new float[positionsCount * numberOfComponents]{});
            positionData = reinterpret_cast<float*>(&_model.buffers[bufferIndex].data[offset + accessorOffset]);
            const size_t elementStride = bytesStride / sizeof(float);

            for (size_t i = 0, j = 0; i < positionsCount * numberOfComponents;) {
                for (size_t k = 0; k < numberOfComponents; ++k)
                    positions[i + k] = positionData[j + k];
                i += numberOfComponents; j += elementStride;
            }

            const size_t indicesAccessorIndex = primitive.indices;
            const size_t indicesBufferViewIndex = _model.accessors[indicesAccessorIndex].bufferView;
            indicesCount = _model.accessors[indicesAccessorIndex].count;
            const size_t indicesAccessorOffset = _model.accessors[indicesAccessorIndex].byteOffset;
            const size_t indicesBufferIndex = _model.bufferViews[indicesBufferViewIndex].buffer;
            const size_t indicesLength = _model.bufferViews[indicesBufferViewIndex].byteLength;
            const size_t indicesOffset = _model.bufferViews[indicesBufferViewIndex].byteOffset;
            const AccessorType indicesAccessorType = static_cast<AccessorType>(_model.accessors[indicesAccessorIndex].type);
            const ComponentType indicesComponentType = static_cast<ComponentType>(_model.accessors[indicesAccessorIndex].componentType);
            const size_t indicesNumberOfComponents = getNumberOfComponents(indicesAccessorType);
            const size_t indicesComponentTypeSize = getComponentTypeSize(indicesComponentType);
            const size_t indicesStride = _model.bufferViews[indicesBufferViewIndex].byteStride / indicesComponentTypeSize;
            indicesData = &_model.buffers[indicesBufferIndex].data[accessorOffset + indicesOffset];

            indices = new uint32_t[indicesCount]{};
            for (size_t i = 0, j = 0; i < indicesCount;) {
                for (size_t k = 0; k < indicesNumberOfComponents; ++k)
                    memcpy(indices + i + k, indicesData + (indicesComponentTypeSize * (j + k)), indicesComponentTypeSize);
                i += indicesNumberOfComponents;
                if (indicesStride > 0)
                    j += indicesStride;
                else
                    j++;
            }

            // Read texture coordinates.
            constexpr char TEXCOORD[] = "TEXCOORD";
            constexpr size_t TEXCOORD_LENGTH = std::size(TEXCOORD) - 1;
            size_t texCoordNumberOfComponents = 0;
            for (const auto &attrIndexPair: attributes) {
                // [cpp20] Replace this by begins_with()
                if (attrIndexPair.first.length() > TEXCOORD_LENGTH && attrIndexPair.first.substr(0, TEXCOORD_LENGTH) == TEXCOORD) {
                    const size_t texCoordAccessorIndex = attributes.at(attrIndexPair.first);
                    texCoordsCount = _model.accessors[texCoordAccessorIndex].count;
                    const size_t texCoordAccessorOffset = _model.accessors[texCoordAccessorIndex].byteOffset;
                    const size_t texCoordBufferViewIndex = _model.accessors[texCoordAccessorIndex].bufferView;
                    const size_t texCoordBufferIndex = _model.bufferViews[texCoordBufferViewIndex].buffer;
                    const size_t length = _model.bufferViews[texCoordBufferViewIndex].byteLength;
                    const size_t texCoordOffset = _model.bufferViews[texCoordBufferViewIndex].byteOffset;
                    const AccessorType texCoordAccessorType = static_cast<AccessorType>(_model.accessors[texCoordAccessorIndex].type);
                    const ComponentType texCoordComponentType = static_cast<ComponentType>(_model.accessors[texCoordAccessorIndex].componentType);
                    texCoordNumberOfComponents = getNumberOfComponents(texCoordAccessorType);
                    const size_t texCoordComponentTypeSize = getComponentTypeSize(texCoordComponentType);
                    texCoords.reset(new float[texCoordsCount * texCoordNumberOfComponents]{});
                    texCoordData = reinterpret_cast<float*>(&_model.buffers[texCoordBufferIndex].data[texCoordAccessorOffset + texCoordOffset]);
                    const size_t texCoordStride = _model.bufferViews[texCoordBufferViewIndex].byteStride / texCoordComponentTypeSize;

                    for (size_t i = 0, j = 0; i < texCoordsCount * texCoordNumberOfComponents;) {
                        for (size_t k = 0; k < texCoordNumberOfComponents; ++k)
                            texCoords[i + k] = texCoordData[j + k];
                        i += texCoordNumberOfComponents; j += texCoordStride;
                    }
                }
            }

            // Form output vertices.
            vertices = new Vertex[positionsCount];
            for (size_t i = 0, positionI = 0, textureCoordinatesI = 0; i < positionsCount; ++i) {
                vertices[i].position.x = positions[positionI];
                vertices[i].position.y = positions[positionI + 1];
                vertices[i].position.z = positions[positionI + 2];
                vertices[i].textureCoordinate.x = texCoords[textureCoordinatesI];
                vertices[i].textureCoordinate.y = texCoords[textureCoordinatesI + 1];
                positionI += numberOfComponents; textureCoordinatesI += texCoordNumberOfComponents;
            }

            // Load textures.
            for (tinygltf::Texture &texture: _model.textures) {
                const int imageIndex = texture.source;
                imageToLoad = &_model.images[imageIndex];

                const int samplerIndex = texture.sampler;
            }
         }
    }


    _vertices.push_back(vertices);
    _verticesCounts.push_back(positionsCount);
    _indices.push_back(indices);
    _indicesCounts.push_back(indicesCount);

    if (nullptr != imageToLoad) {
        auto [image, imageView, sampler] = loadTexture(_swapchain, _commandPool, _graphicsQueue, *imageToLoad);
        _images.emplace_back(std::move(image));
        _imageViews.emplace_back(std::move(imageView));
        _samplers.push_back(std::move(sampler));
    }
    //return {vertices, positionsCount, indices, indicesCount, imageToLoad};
    return 0;
}

//void unloadModel(const uint32_t index);
//
std::pair<avocado::Vertex*, size_t /* count */> ModelLoader::getVertices(const uint32_t index) {
    return {_vertices[index], _verticesCounts[index]};
}

std::pair<uint32_t*, size_t /* count */> ModelLoader::getIndices(const uint32_t index) {
    return {_indices[index], _indicesCounts[index]};
}
avocado::vulkan::Image& ModelLoader::getImage(const uint32_t index) {
    return _images[index];
}

avocado::vulkan::ImageViewPtr& ModelLoader::getImageView(const uint32_t index) {
    return _imageViews[index];
}

avocado::vulkan::SamplerPtr& ModelLoader::getSampler(const uint32_t index) {
    return _samplers[index];
}

std::tuple<avocado::vulkan::Image, avocado::vulkan::ImageViewPtr, avocado::vulkan::SamplerPtr> ModelLoader::loadTexture(avocado::vulkan::Swapchain &swapChain,
        avocado::vulkan::CommandPool &commandPool, avocado::vulkan::Queue &graphicsQueue, const tinygltf::Image &image) {
    const auto &imagePixels = image.image;
    vulkan::Buffer imgBuffer(imagePixels.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, _logicalDevice);
    imgBuffer.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    imgBuffer.fill(imagePixels.data());
    imgBuffer.bindMemory();

    // Create image.
    vulkan::Image textureImage(_logicalDevice, image.width, image.height, VK_IMAGE_TYPE_2D);
    textureImage.setDepth(1);
    textureImage.setFormat(VK_FORMAT_R8G8B8A8_SRGB);
    textureImage.setMipLevels(1);
    textureImage.setImageTiling(VK_IMAGE_TILING_OPTIMAL);
    textureImage.setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    textureImage.setSampleCount(VK_SAMPLE_COUNT_1_BIT);
    textureImage.setArrayLayerCount(1);
    textureImage.setSharingMode(VK_SHARING_MODE_EXCLUSIVE);
    textureImage.create();
    if (textureImage.hasError()) {
        return std::make_tuple(std::move(textureImage), _logicalDevice.createObjectPointer<VkImageView>(VK_NULL_HANDLE),
            _logicalDevice.createObjectPointer<VkSampler>(VK_NULL_HANDLE));
    }

    textureImage.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    textureImage.bindMemory();

    vulkan::ImageViewPtr textureImageView = _logicalDevice.createObjectPointer(swapChain.createImageView(textureImage.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));

    // Copy buffer to image.
    vulkan::CommandBuffer cmdBufFor1stTransition = commandPool.getBuffer(static_cast<size_t>(BufferIndex::FirstImageLayoutTransition));
    cmdBufFor1stTransition.reset(VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    avocado::vulkan::transitImageLayout(cmdBufFor1stTransition, graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
    vulkan::copyBufferToImage(commandPool, graphicsQueue, imgBuffer, textureImage, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));
    vulkan::CommandBuffer cmdBufFor2ndTransition = commandPool.getBuffer(static_cast<size_t>(BufferIndex::SecondImageLayoutTransition));
    cmdBufFor2ndTransition.reset(VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    avocado::vulkan::transitImageLayout(cmdBufFor2ndTransition, graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

    vulkan::SamplerPtr textureSamplerPtr = _logicalDevice.createSampler(_physicalDevice);
    if (_logicalDevice.hasError()) {
        return std::make_tuple(std::move(textureImage), _logicalDevice.createObjectPointer<VkImageView>(VK_NULL_HANDLE),
            _logicalDevice.createObjectPointer<VkSampler>(VK_NULL_HANDLE));
    }

    return std::make_tuple(std::move(textureImage), std::move(textureImageView), std::move(textureSamplerPtr));

}

}

