#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define JSON_NOEXCEPTION

#include "scene.hpp"

#include "vulkan/logicaldevice.hpp"
#include "vulkan/swapchain.hpp"


#include <iostream> // todo remove
#include <algorithm>
#include <memory>
#include <functional>

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

} // anonymous namespace.

// todo remove
struct VulkanObjectFactory {
    explicit VulkanObjectFactory(avocado::vulkan::PhysicalDevice &physicalDevice, avocado::vulkan::LogicalDevice &logicalDevice, avocado::vulkan::Swapchain &swapchain):
        _physicalDevice(physicalDevice),
        _logicalDevice(logicalDevice),
        _swapchain(swapchain) {
    }

    avocado::vulkan::Image createImage(const avocado::core::Image &sceneImage) {
        avocado::vulkan::Image textureImage(_logicalDevice, sceneImage._width, sceneImage._height, VK_IMAGE_TYPE_2D);
        textureImage.setDepth(1);
        textureImage.setFormat(VK_FORMAT_R8G8B8A8_SRGB);
        textureImage.setMipLevels(1);
        textureImage.setImageTiling(VK_IMAGE_TILING_OPTIMAL);
        textureImage.setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        textureImage.setSampleCount(VK_SAMPLE_COUNT_1_BIT);
        textureImage.setArrayLayerCount(1);
        textureImage.setSharingMode(VK_SHARING_MODE_EXCLUSIVE);
        textureImage.create();
        // todo Log this error
        //if (textureImage.hasError()) {
        //}
        textureImage.allocateMemory(_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        textureImage.bindMemory();
        return textureImage;
    }



    avocado::vulkan::ImageViewPtr createImageView(avocado::vulkan::Image image){
        return _logicalDevice.createObjectPointer(_swapchain.createImageView(image.getHandle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));
    }

    avocado::vulkan::SamplerPtr createSampler() {
        return _logicalDevice.createSampler(_physicalDevice);
    }

    avocado::vulkan::PhysicalDevice &_physicalDevice;
    avocado::vulkan::LogicalDevice &_logicalDevice;
    avocado::vulkan::Swapchain &_swapchain;
};


namespace avocado::core {

    //// Node class
    void Node::addChild(Node * const childNode) {
    if (nullptr != childNode)
        _children.push_back(childNode);
}

bool Node::hasMatrix() const noexcept {
    return (nullptr != matrix);
}

bool Node::hasMesh() const noexcept {
    return (nullptr != mesh);
}

bool Node::hasRotation() const noexcept {
    return (nullptr != rotation);
}


//// Sampler class
VkSamplerCreateInfo Sampler::generateVulkanCreateInfo() const {
    VkSamplerCreateInfo createInfo{};
    createInfo.minFilter = static_cast<VkFilter>(_minFilter);
    createInfo.magFilter = static_cast<VkFilter>(_magFilter);
    createInfo.addressModeU = static_cast<VkSamplerAddressMode>(_wrapS);
    createInfo.addressModeV = static_cast<VkSamplerAddressMode>(_wrapT);
    return createInfo;
}


//// SceneManager class.
void SceneManager::load(const std::string &filepath) {
    auto loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadASCIIFromFile);
    if (filepath.ends_with(".glb"))
        loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadBinaryFromFile);


    tinygltf::TinyGLTF loader;
    std::string error, warning;
    bool loadOk = false;
    loadOk = loadFunction(loader, &model, &error, &warning, filepath, tinygltf::REQUIRE_VERSION);
    if (!loadOk) {
        ; // todo Process error.
    }

    std::unique_ptr<float[]> positions = nullptr; size_t positionsCount = 0;
    std::unique_ptr<float[]> texCoords = nullptr; size_t texCoordsCount = 0;
    std::unique_ptr<float[]> colors = nullptr; size_t colorCount = 0;
    uint32_t *indices = nullptr; size_t indicesCount = 0;
    float *positionData = nullptr;
    unsigned char *indicesData = nullptr;
    float *texCoordData = nullptr;
    float *colorData = nullptr;
    Vertex *vertices = nullptr;
    tinygltf::Image *imageToLoad = nullptr;
    int meshIndex = 0;
    for (const tinygltf::Mesh &mesh: model.meshes) {
        Mesh newMesh;
        for (const tinygltf::Primitive &primitive: mesh.primitives) {
            const ModelTopology topology = static_cast<ModelTopology>(primitive.mode);
            const auto &attributes = primitive.attributes;
            const size_t accessorIndex = attributes.at("POSITION");
            positionsCount = model.accessors[accessorIndex].count;
            const size_t accessorOffset = model.accessors[accessorIndex].byteOffset;
            const size_t bufferViewIndex = model.accessors[accessorIndex].bufferView;
            const size_t bufferIndex = model.bufferViews[bufferViewIndex].buffer;
            const size_t length = model.bufferViews[bufferViewIndex].byteLength;
            const size_t offset = model.bufferViews[bufferViewIndex].byteOffset;
            const size_t bytesStride = model.bufferViews[bufferViewIndex].byteStride;

            const AccessorType accessorType = static_cast<AccessorType>(model.accessors[accessorIndex].type);
            const size_t numberOfComponents = getNumberOfComponents(accessorType);

            positions.reset(new float[positionsCount * numberOfComponents]{});
            positionData = reinterpret_cast<float*>(&model.buffers[bufferIndex].data[offset + accessorOffset]);
            const size_t elementStride = bytesStride / sizeof(float);

            for (size_t i = 0, j = 0; i < positionsCount * numberOfComponents;) {
                for (size_t k = 0; k < numberOfComponents; ++k)
                    positions[i + k] = positionData[j + k];
                i += numberOfComponents;
                if (0 == elementStride)
                    j += numberOfComponents;
                else
                    j += elementStride;
            }

            const size_t indicesAccessorIndex = primitive.indices;
            const size_t indicesBufferViewIndex = model.accessors[indicesAccessorIndex].bufferView;
            indicesCount = model.accessors[indicesAccessorIndex].count;
            const size_t indicesAccessorOffset = model.accessors[indicesAccessorIndex].byteOffset;
            const size_t indicesBufferIndex = model.bufferViews[indicesBufferViewIndex].buffer;
            const size_t indicesLength = model.bufferViews[indicesBufferViewIndex].byteLength;
            const size_t indicesOffset = model.bufferViews[indicesBufferViewIndex].byteOffset;
            const AccessorType indicesAccessorType = static_cast<AccessorType>(model.accessors[indicesAccessorIndex].type);
            const ComponentType indicesComponentType = static_cast<ComponentType>(model.accessors[indicesAccessorIndex].componentType);
            const size_t indicesNumberOfComponents = getNumberOfComponents(indicesAccessorType);
            const size_t indicesComponentTypeSize = getComponentTypeSize(indicesComponentType);
            const size_t indicesStride = model.bufferViews[indicesBufferViewIndex].byteStride / indicesComponentTypeSize;
            indicesData = &model.buffers[indicesBufferIndex].data[indicesAccessorOffset + indicesOffset];

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
                    texCoordsCount = model.accessors[texCoordAccessorIndex].count;
                    const size_t texCoordAccessorOffset = model.accessors[texCoordAccessorIndex].byteOffset;
                    const size_t texCoordBufferViewIndex = model.accessors[texCoordAccessorIndex].bufferView;
                    const size_t texCoordBufferIndex = model.bufferViews[texCoordBufferViewIndex].buffer;
                    const size_t length = model.bufferViews[texCoordBufferViewIndex].byteLength;
                    const size_t texCoordOffset = model.bufferViews[texCoordBufferViewIndex].byteOffset;
                    const AccessorType texCoordAccessorType = static_cast<AccessorType>(model.accessors[texCoordAccessorIndex].type);
                    const ComponentType texCoordComponentType = static_cast<ComponentType>(model.accessors[texCoordAccessorIndex].componentType);
                    texCoordNumberOfComponents = getNumberOfComponents(texCoordAccessorType);
                    const size_t texCoordComponentTypeSize = getComponentTypeSize(texCoordComponentType);
                    texCoords.reset(new float[texCoordsCount * texCoordNumberOfComponents]{});
                    texCoordData = reinterpret_cast<float*>(&model.buffers[texCoordBufferIndex].data[texCoordAccessorOffset + texCoordOffset]);
                    const size_t texCoordStride = model.bufferViews[texCoordBufferViewIndex].byteStride / texCoordComponentTypeSize;

                    for (size_t i = 0, j = 0; i < texCoordsCount * texCoordNumberOfComponents;) {
                        for (size_t k = 0; k < texCoordNumberOfComponents; ++k)
                            texCoords[i + k] = texCoordData[j + k];
                        i += texCoordNumberOfComponents;
                        if (0 == texCoordStride)
                            j+= texCoordNumberOfComponents;
                        else
                            j += texCoordStride;
                    }
                }
            }

            // Read colors.
            constexpr char COLOR[] = "COLOR";
            constexpr size_t COLOR_LENGTH = std::size(COLOR) - 1;
            size_t colorNumberOfComponents = 0;
            for (const auto &attrIndexPair: attributes) {
                // [cpp20] Replace this by begins_with()
                if (attrIndexPair.first.length() > COLOR_LENGTH && attrIndexPair.first.substr(0, COLOR_LENGTH) == COLOR) {
                    const size_t colorAccessorIndex = attributes.at(attrIndexPair.first);
                    colorCount = model.accessors[colorAccessorIndex].count;
                    const size_t colorAccessorOffset = model.accessors[colorAccessorIndex].byteOffset;
                    const size_t colorBufferViewIndex = model.accessors[colorAccessorIndex].bufferView;
                    const size_t colorBufferIndex = model.bufferViews[colorBufferViewIndex].buffer;
                    const size_t length = model.bufferViews[colorBufferViewIndex].byteLength;
                    const size_t colorOffset = model.bufferViews[colorBufferViewIndex].byteOffset;
                    const AccessorType colorAccessorType = static_cast<AccessorType>(model.accessors[colorAccessorIndex].type);
                    const ComponentType colorComponentType = static_cast<ComponentType>(model.accessors[colorAccessorIndex].componentType);
                    colorNumberOfComponents = getNumberOfComponents(colorAccessorType);
                    const size_t colorComponentTypeSize = getComponentTypeSize(colorComponentType);
                    colors.reset(new float[colorCount * colorNumberOfComponents]{});
                    colorData = reinterpret_cast<float*>(&model.buffers[colorBufferIndex].data[colorAccessorOffset + colorOffset]);
                    const size_t colorStride = model.bufferViews[colorBufferViewIndex].byteStride / colorComponentTypeSize;

                    for (size_t i = 0, j = 0; i < colorCount * colorNumberOfComponents;) {
                        for (size_t k = 0; k < colorNumberOfComponents; ++k)
                            colors[i + k] = colorData[j + k];
                        i += colorNumberOfComponents;
                        if (0 == colorStride)
                            j+= colorNumberOfComponents;
                        else
                            j += colorStride;
                    }
                }
            }


            // Form output vertices.
            vertices = new Vertex[positionsCount];
            for (size_t i = 0, positionI = 0, colorI = 0, textureCoordinatesI = 0; i < positionsCount; ++i) {
                vertices[i].position.x = positions[positionI];
                vertices[i].position.y = positions[positionI + 1];
                vertices[i].position.z = positions[positionI + 2];
                vertices[i].textureCoordinate.x = texCoords[textureCoordinatesI];
                vertices[i].textureCoordinate.y = texCoords[textureCoordinatesI + 1];
                if (nullptr != colors) {
                    vertices[i].color.r = colors[colorI];
                    vertices[i].color.g = colors[colorI + 1];
                    vertices[i].color.b = colors[colorI + 2];
                } else { // Paint as red by default.
                    if (0 == meshIndex) {
                        vertices[i].color.r = 16.f / 255.f;
                        vertices[i].color.g = 103.f / 255.f;
                        vertices[i].color.b = 57.f / 255.f;
                    } else {
//148, 73, 23
                        vertices[i].color.r = 148.f / 255.f;
                        vertices[i].color.g = 73.f / 255.f;
                        vertices[i].color.b = 23.f / 255.f;

                    }
                }
                positionI += numberOfComponents; textureCoordinatesI += texCoordNumberOfComponents;
            }

            parseSamplers();
            parseImages();
            parseTextures();

            // todo Could we reserve here anything for each primitive?
            for (size_t i = 0; i < positionsCount; ++i)
                newMesh.vertices.push_back(std::move(vertices[i]));
            for (size_t i = 0; i < indicesCount; ++i)
                newMesh.indices.emplace_back(indices[i]);
        } // for each primitive.
        meshes.push_back(std::move(newMesh)); 
        meshIndex++;
    } // for each mesh.

    parseNodes();
    parseScenes();
}

void SceneManager::parseSamplers() {
    for (const tinygltf::Sampler &sampler: model.samplers) {
        Sampler newSampler;
        
        switch (sampler.magFilter) {
            case TINYGLTF_TEXTURE_FILTER_LINEAR: {
                newSampler._magFilter = Sampler::MagFilter::Linear;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST: {
                newSampler._magFilter = Sampler::MagFilter::Nearest;
                break;
            }
            case -1: {
                newSampler._magFilter = Sampler::MagFilter::None;
                break;
            }
            default: {
                assert("Unknown mag filter type");
            }
        }

        switch (sampler.minFilter) {
            case TINYGLTF_TEXTURE_FILTER_LINEAR: {
                newSampler._minFilter = Sampler::MinFilter::Linear;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST: {
                newSampler._minFilter = Sampler::MinFilter::Nearest;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: {
                newSampler._minFilter = Sampler::MinFilter::NearestMipMapNearest;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: {
                newSampler._minFilter = Sampler::MinFilter::NearestMipMapLinear;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: {
                newSampler._minFilter = Sampler::MinFilter::LinearMipMapNearest;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: {
                newSampler._minFilter = Sampler::MinFilter::LinearMipMapLinear;
                break;
            }
            case -1: {
                newSampler._minFilter = Sampler::MinFilter::None;
                break;
            }
            default: {
                assert("Unknown min filter type");
            }
        }

        switch (sampler.wrapT) {
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: {
               newSampler._wrapT = Sampler::TextureWrap::ClampToEdge;
               break;
            }
            case TINYGLTF_TEXTURE_WRAP_REPEAT: {
               newSampler._wrapT = Sampler::TextureWrap::Repeat;
               break;
            }
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: {
               newSampler._wrapT = Sampler::TextureWrap::MirroredRepeat;
               break;
            }
            default:
                assert("Unknown texture wrapT type");
        }

        switch (sampler.wrapS) {
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: {
               newSampler._wrapS = Sampler::TextureWrap::ClampToEdge;
               break;
            }
            case TINYGLTF_TEXTURE_WRAP_REPEAT: {
               newSampler._wrapS = Sampler::TextureWrap::Repeat;
               break;
            }
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: {
               newSampler._wrapS = Sampler::TextureWrap::MirroredRepeat;
               break;
            }
            default:
                assert("Unknown texture wrapS type");
        }

        _samplers.push_back(std::move(newSampler));
    }
}

void SceneManager::parseImages() {
    for (tinygltf::Image &image: model.images) {
        std::cout << "IMAGE: " << image.name << std::endl;
        Image newImage;
        if (!image.uri.empty())
            newImage._uri = image.uri;

        newImage._width = image.width;
        newImage._height = image.height;
        if (!image.mimeType.empty()) {
            if ("image/png" == image.mimeType)
                newImage._mimeType = Image::MimeType::Png;
            else if ("image/jpg" == image.mimeType || "image/jpeg" == image.mimeType)
                newImage._mimeType = Image::MimeType::Jpg;
            else if ("image/bmp" == image.mimeType)
                newImage._mimeType = Image::MimeType::Bmp;
            else if ("image/gif" == image.mimeType)
                newImage._mimeType = Image::MimeType::Gif;
        }

        if (image.uri.empty() && image.bufferView != -1) {
            const tinygltf::BufferView &bufferView = model.bufferViews[image.bufferView];
            tinygltf::Buffer buffer = model.buffers[bufferView.buffer];
            newImage._data = std::move(buffer.data);
        } else {
            newImage._data = std::move(image.image);
        }

        _images.push_back(std::move(newImage));
    }
}


void SceneManager::parseTextures() {
    for (const tinygltf::Texture &texture: model.textures) {
        Texture newTexture;
        
        std::cout << "TEXTURE: " << texture.name << std::endl;
        if (-1 != texture.source)
            newTexture._image = &_images[texture.source];
        if (-1 != texture.sampler)
            newTexture._sampler = &_samplers[texture.sampler];

        _textures.push_back(std::move(newTexture));
    }
}

void foo(const Node &node, std::vector<Vertex> &totalVertices) {
    if (node.hasMesh())
        std::copy(node.mesh->vertices.begin(), node.mesh->vertices.end(), std::back_inserter(totalVertices));

    for (const Node * const childNode: node._children)
        foo(*childNode, totalVertices);
}

avocado::vulkan::Buffer SceneManager::formVertexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device) {
    if (nullptr == _defaultScene)
        return avocado::vulkan::Buffer(0, usage, sharingMode, device);

    std::vector<Vertex> totalVertices;
    for (const Node * const rootNode: _defaultScene->_rootNodes)
        foo(*rootNode, totalVertices);

    const size_t sizeBytes = sizeof(decltype(totalVertices)::value_type) * totalVertices.size();
    avocado::vulkan::Buffer buffer(sizeBytes, usage, sharingMode, device);
    buffer.allocateMemory(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    buffer.fill(totalVertices.data());
    buffer.bindMemory();
    return buffer;
}

void bar(const Node &node, std::vector<uint32_t> &totalIndices) {
    if (node.hasMesh()) {
        auto max_element = std::max_element(totalIndices.cbegin(), totalIndices.cend());
        uint32_t offset = 0;
        if (max_element != totalIndices.cend())
            offset = *max_element + 1;

        for (size_t j = 0; j < node.mesh->indices.size(); ++j)
            totalIndices.push_back(node.mesh->indices[j] + offset);
    }

    for (const Node * const childNode: node._children)
        bar(*childNode, totalIndices);
}

avocado::vulkan::Buffer SceneManager::formIndexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device) {
    if (nullptr == _defaultScene)
        return avocado::vulkan::Buffer(0, usage, sharingMode, device);

    std::vector<uint32_t> totalIndices;
    for (size_t i = 0; i < _defaultScene->_rootNodes.size(); ++i) {
        const Node * const rootNode = _defaultScene->_rootNodes[i];
        bar(*rootNode, totalIndices);
    }

    const size_t sizeBytes = sizeof(decltype(totalIndices)::value_type) * totalIndices.size();
    avocado::vulkan::Buffer buffer(sizeBytes, usage, sharingMode, device);
    buffer.allocateMemory(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    buffer.fill(totalIndices.data());
    buffer.bindMemory();
    return buffer;
}

void SceneManager::parseNodes() {
    for (const tinygltf::Node &node: model.nodes) {
        Node newNode;
        newNode._name = node.name;
        if (-1 != node.mesh) {
            assert(node.mesh < meshes.size());
            newNode.mesh = &meshes[node.mesh];
        }

        if (!node.matrix.empty()) {
            newNode.matrix = new math::Mat4x4;
            size_t i = 0;
            for (size_t column = 0; column < 4; ++column) {
                for (size_t row = 0; row < 4; ++row, ++i) {
                    (*newNode.matrix)[row][column] = node.matrix[i];
                }
            }
        }

        if (!node.rotation.empty()) {
            newNode.rotation = new math::Quaternion{
                static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2]), static_cast<float>(node.rotation[3])};
        }

        if (!node.translation.empty()) {
            newNode.translation = new math::vec3f;
            newNode.translation->x = node.translation[0];
            newNode.translation->y = node.translation[1];
            newNode.translation->z = node.translation[2];
        }

        using namespace avocado::math;
        // mesh * vec
        if (newNode.hasMatrix() && newNode.hasMesh()) {
            for (avocado::Vertex &vertex: newNode.mesh->vertices) {
                math::vec4f v(1.f, 1.f, 1.f, 1.f);
                v.x = vertex.position.x;
                v.y = vertex.position.y;
                v.z = vertex.position.z;
                v = v * (*newNode.matrix);

                vertex.position.x = v.x;                
                vertex.position.y = v.y;
                vertex.position.z = v.z;
            }
        }

        if (newNode.translation != nullptr && newNode.hasMesh()) {
            for (avocado::Vertex &vertex: newNode.mesh->vertices)
                vertex.position = vertex.position + (*newNode.translation);
        }

        nodes.push_back(std::move(newNode));
    }

    for (size_t i = 0; i < model.nodes.size(); ++i) {
        for (const int childIndex: model.nodes[i].children)
            nodes[i].addChild(&nodes[childIndex]);
    }
    _nodesAreParsed = true;
}

void SceneManager::parseScenes() {
    assert(_nodesAreParsed);

    for (tinygltf::Scene &scene: model.scenes) {
        Scene newScene;
        newScene._name = scene.name;
        for (const int nodeIndex: scene.nodes) {
            assert(nodeIndex < nodes.size());
            newScene._rootNodes.push_back(&nodes[nodeIndex]);
        }
        _scenes.push_back(std::move(newScene));
    }

    assert(model.defaultScene < _scenes.size());
    _defaultScene = &_scenes[model.defaultScene];
}

} // namespace avocado::core

