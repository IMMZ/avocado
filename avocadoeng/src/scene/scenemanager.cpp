#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "scenemanager.hpp"

#include "../math/functions.hpp"

#include <functional>

namespace avocado::core {

void SceneManager::load(const std::string &filepath) {
    auto loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadASCIIFromFile);
    if (filepath.ends_with(".glb"))
        loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadBinaryFromFile);


    tinygltf::TinyGLTF loader;
    std::string error, warning;
    const bool loadOk = loadFunction(loader, &model, &error, &warning, filepath, tinygltf::REQUIRE_VERSION);
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
            constexpr const char * const TEXCOORD = "TEXCOORD";
            size_t texCoordNumberOfComponents = 0;
            for (const auto &attrIndexPair: attributes) {
                if (attrIndexPair.first.starts_with(TEXCOORD)) {
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

            // todo Looks like this code is similar to read texture coordinates. Refactor!
            // Read colors.
            constexpr char COLOR[] = "COLOR";
            size_t colorNumberOfComponents = 0;
            for (const auto &attrIndexPair: attributes) {
                if (attrIndexPair.first.starts_with(COLOR)) {
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
                if (nullptr != texCoords) {
                    vertices[i].textureCoordinate.x = texCoords[textureCoordinatesI];
                    vertices[i].textureCoordinate.y = texCoords[textureCoordinatesI + 1];
                }

                if (nullptr != colors) {
                    vertices[i].color.r = colors[colorI];
                    vertices[i].color.g = colors[colorI + 1];
                    vertices[i].color.b = colors[colorI + 2];
                    // todo Decide if we need this 4th alpha component of color.
                    //if (4 == colorNumberOfComponents)
                    //    vertices[i].color.a = colors[colorI + 3];

                } else { // Paint as red by default.
                    vertices[i].color.r = 1.f;
                    vertices[i].color.g = 0.f;
                    vertices[i].color.b = 0.f;
                }

                positionI += numberOfComponents; textureCoordinatesI += texCoordNumberOfComponents;
            }

            // todo Could we reserve here anything for each primitive?
            for (size_t i = 0; i < positionsCount; ++i)
                newMesh.vertices.push_back(std::move(vertices[i]));
            for (size_t i = 0; i < indicesCount; ++i)
                newMesh.indices.emplace_back(indices[i]);
        } // for each primitive.
        meshes.push_back(std::move(newMesh));
        meshIndex++;
    } // for each mesh.

    parseSamplers();
    parseImages();
    parseTextures();
    parseCameras();
    parseNodes();
    parseScenes();
}

void SceneManager::parseSamplers() {
    for (const tinygltf::Sampler &sampler: model.samplers) {
        Sampler newSampler;

        switch (sampler.magFilter) {
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: {
                newSampler._magFilter = VK_FILTER_LINEAR;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: {
                newSampler._magFilter = VK_FILTER_NEAREST;
                break;
            }
            case -1: {
                newSampler._magFilter = VK_FILTER_MAX_ENUM;
                break;
            }
            default: {
                assert("Unknown mag filter type");
            }
        }

        switch (sampler.minFilter) {
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: {
                newSampler._minFilter = VK_FILTER_LINEAR;
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: {
                newSampler._minFilter = VK_FILTER_NEAREST;
                break;
            }
            case -1: {
                newSampler._minFilter = VK_FILTER_MAX_ENUM;
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

        newImage._data = std::move(image.image);
        _images.push_back(std::move(newImage));
    }
}


void SceneManager::parseTextures() {
    for (const tinygltf::Texture &texture: model.textures) {
        Texture newTexture;

        if (-1 != texture.source)
            newTexture._image = &_images[texture.source];
        if (-1 != texture.sampler)
            newTexture._sampler = &_samplers[texture.sampler];

        _textures.push_back(std::move(newTexture));
    }
}

// todo Mustn't be as a free function.
void copyVerticesFromNode(const Node &node, std::vector<Vertex> &totalVertices) {
    if (node.hasMesh())
        std::copy(node.mesh->vertices.begin(), node.mesh->vertices.end(), std::back_inserter(totalVertices));

    for (const Node * const childNode: node._children)
        copyVerticesFromNode(*childNode, totalVertices);
}

avocado::vulkan::Buffer SceneManager::formVertexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device) {
    if (nullptr == _defaultScene)
        return avocado::vulkan::Buffer(0, usage, sharingMode, device);

    std::vector<Vertex> totalVertices;
    for (const Node * const rootNode: _defaultScene->_rootNodes)
        copyVerticesFromNode(*rootNode, totalVertices);

    const size_t sizeBytes = sizeof(decltype(totalVertices)::value_type) * totalVertices.size();
    avocado::vulkan::Buffer buffer(sizeBytes, usage, sharingMode, device);
    buffer.allocateMemory(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    buffer.fill(totalVertices.data());
    buffer.bindMemory();
    return buffer;
}

// todo Mustn't be as a free function.
void copyIndiciesFromNode(const Node &node, std::vector<uint32_t> &totalIndices) {
    if (node.hasMesh()) {
        auto max_element = std::max_element(totalIndices.cbegin(), totalIndices.cend());
        uint32_t offset = 0;
        if (max_element != totalIndices.cend())
            offset = *max_element + 1;

        for (size_t j = 0; j < node.mesh->indices.size(); ++j)
            totalIndices.push_back(node.mesh->indices[j] + offset);
    }

    for (const Node * const childNode: node._children)
        copyIndiciesFromNode(*childNode, totalIndices);
}

avocado::vulkan::Buffer SceneManager::formIndexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device) {
    if (nullptr == _defaultScene)
        return avocado::vulkan::Buffer(0, usage, sharingMode, device);

    std::vector<uint32_t> totalIndices;
    for (size_t i = 0; i < _defaultScene->_rootNodes.size(); ++i) {
        const Node * const rootNode = _defaultScene->_rootNodes[i];
        copyIndiciesFromNode(*rootNode, totalIndices);
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
        newNode._cameraIndex = node.camera;


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

        if (!node.scale.empty()) {
            newNode.scale = new math::vec3f;
            newNode.scale->x = node.scale[0];
            newNode.scale->y = node.scale[1];
            newNode.scale->z = node.scale[2];
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

        // For row vectors the order is: scale, rotation, translation.
        if (newNode.hasScale()) {
            for (avocado::Vertex &vertex: newNode.mesh->vertices) {
                vertex.position.x *= newNode.scale->x;
                vertex.position.y *= newNode.scale->y;
                vertex.position.z *= newNode.scale->z;
            }
        }

        if (!newNode.hasCamera() && newNode.hasRotation()) {
            for (avocado::Vertex &vertex: newNode.mesh->vertices) {
                vertex.position = newNode.rotation->rotateVector(vertex.position);
            }
        }

        if (newNode.hasMesh() && newNode.hasTranslation()) {
            for (avocado::Vertex &vertex: newNode.mesh->vertices)
                vertex.position = vertex.position + (*newNode.translation);
        }

        if (newNode.hasCamera()) {
            if (newNode.hasTranslation())
                _cameras[newNode._cameraIndex].position = (*newNode.translation);

            if (newNode.hasRotation()) {
                _cameras[newNode._cameraIndex].up = newNode.rotation->rotateVector(_cameras[newNode._cameraIndex].up);
            }
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

void SceneManager::parseCameras() {
    for (const tinygltf::Camera &camera: model.cameras) {
        assert(camera.type == "perspective"); //No support for non-perspective projective on camera.

        _cameras.push_back(Camera {
            .perspective = math::perspectiveProjection(
                static_cast<float>(math::toDegrees(camera.perspective.yfov)),
                static_cast<float>(camera.perspective.aspectRatio),
                static_cast<float>(camera.perspective.znear),
                static_cast<float>(camera.perspective.zfar)),
            .up = math::vec3f{0.f, 1.f, 0.f}});

    }
}

}
