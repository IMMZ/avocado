#include "vulkan/vulkan_core.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "scenemanager.hpp"

#include "../logger.hpp"
#include "../math/functions.hpp"
#include "../math/matrix.hpp"
#include "../math/quaternion.hpp"

#include <functional>
#include <stack>

namespace {
    constexpr int NODE_INDEX_FOR_NO_PARENT = -1;
}

namespace avocado::core {

void SceneManager::calculateMatrixForNode(const int nodeIndex, const int parentNodeIndex) {
    if (-1 == nodeIndex)
        return;

    const tinygltf::Node &node = _model.nodes[nodeIndex];

    math::Mat4x4 modelMatrix;
    if (!node.matrix.empty())
        modelMatrix = avocado::math::Mat4x4::fromTinyGltf(node.matrix);
    else if (!node.scale.empty() || !node.rotation.empty() || !node.translation.empty())
        modelMatrix = avocado::math::Mat4x4::fromSRT(node.scale, node.rotation, node.translation);

    if (parentNodeIndex != NODE_INDEX_FOR_NO_PARENT)
        modelMatrix *= _nodeMatrices[parentNodeIndex];

    _nodeMatrices[nodeIndex] = std::move(modelMatrix);
}

void SceneManager::calculateMatricesForNode(const tinygltf::Scene &scene) {
    _nodeMatrices.resize(scene.nodes.size());
    using NodeIndexParentIndex = std::pair<int /* node */, int /* parent */>;
    std::stack<NodeIndexParentIndex> nodeIndexStack;
    for (const int nodeIndex: scene.nodes)
        nodeIndexStack.push(NodeIndexParentIndex(nodeIndex, NODE_INDEX_FOR_NO_PARENT));

    while (!nodeIndexStack.empty()) {
        const NodeIndexParentIndex processedNodeIndex = nodeIndexStack.top();
        nodeIndexStack.pop();
        calculateMatrixForNode(processedNodeIndex.first, processedNodeIndex.second);
        const tinygltf::Node &nodeToProcess = _model.nodes[processedNodeIndex.first];
        for (const int childIndex: nodeToProcess.children)
            nodeIndexStack.push(NodeIndexParentIndex(childIndex, processedNodeIndex.second));
    }
}


void SceneManager::load(const std::string &filepath) {
    auto loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadASCIIFromFile);
    if (filepath.ends_with(".glb"))
        loadFunction = std::mem_fn(&tinygltf::TinyGLTF::LoadBinaryFromFile);

    tinygltf::TinyGLTF loader;
    std::string error, warning;
    const bool loadOk = loadFunction(loader, &_model, &error, &warning, filepath, tinygltf::REQUIRE_VERSION);
    if (!loadOk) {
        if (!error.empty())
            LOG_ERROR(error);
        else if (!warning.empty())
            LOG_WARNING(warning);
    }

    parseSamplers();
    parseImages();
    parseTextures();
    parseCameras();
    parseNodes();
    parseScenes();


    _modelMatrix = math::Mat4x4::createIdentityMatrix();

    //for (const tinygltf::Scene &scene: _model.scenes)
    //    calculateMatricesForNode(scene);

    _primitivesCount = std::accumulate(_model.meshes.cbegin(), _model.meshes.cend(), 0,
        [](const int32_t init, const tinygltf::Mesh &mesh) {
            return (init + mesh.primitives.size());
        });
}

void SceneManager::parseSamplers() {
    for (const tinygltf::Sampler &sampler: _model.samplers) {
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
    for (tinygltf::Image &image: _model.images) {
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
    for (const tinygltf::Texture &texture: _model.textures) {
        Texture newTexture;

        if (-1 != texture.source)
            newTexture._image = &_images[texture.source];
        if (-1 != texture.sampler)
            newTexture._sampler = &_samplers[texture.sampler];

        _textures.push_back(std::move(newTexture));
    }
}

std::vector<Vertex> SceneManager::copyVerticesFromPrimitive(const tinygltf::Primitive &primitive) {
    const auto &attributes = primitive.attributes;
    const size_t accessorIndex = attributes.at("POSITION");
    size_t positionsCount = _model.accessors[accessorIndex].count;
    const size_t accessorOffset = _model.accessors[accessorIndex].byteOffset;
    const size_t bufferViewIndex = _model.accessors[accessorIndex].bufferView;
    const size_t bufferIndex = _model.bufferViews[bufferViewIndex].buffer;
    const size_t length = _model.bufferViews[bufferViewIndex].byteLength;
    const size_t offset = _model.bufferViews[bufferViewIndex].byteOffset;
    const size_t bytesStride = _model.bufferViews[bufferViewIndex].byteStride;

    const AccessorType accessorType = static_cast<AccessorType>(_model.accessors[accessorIndex].type);
    const size_t numberOfComponents = getNumberOfComponents(accessorType);

    std::unique_ptr<float[]> positions(new float[positionsCount * numberOfComponents]);
    float * const positionData = reinterpret_cast<float*>(&_model.buffers[bufferIndex].data[offset + accessorOffset]);
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

    // Read texture coordinates.
    constexpr const char * const TEXCOORD = "TEXCOORD";
    size_t texCoordNumberOfComponents = 0;
    std::unique_ptr<float[]> texCoords = nullptr;
    for (const auto &attrIndexPair: attributes) {
        if (attrIndexPair.first.starts_with(TEXCOORD)) {
            const size_t texCoordAccessorIndex = attributes.at(attrIndexPair.first);
            size_t texCoordsCount = _model.accessors[texCoordAccessorIndex].count;
            const size_t texCoordAccessorOffset = _model.accessors[texCoordAccessorIndex].byteOffset;
            const size_t texCoordBufferViewIndex = _model.accessors[texCoordAccessorIndex].bufferView;
            const size_t texCoordBufferIndex = _model.bufferViews[texCoordBufferViewIndex].buffer;
            const size_t length = _model.bufferViews[texCoordBufferViewIndex].byteLength;
            const size_t texCoordOffset = _model.bufferViews[texCoordBufferViewIndex].byteOffset;
            const AccessorType texCoordAccessorType = static_cast<AccessorType>(_model.accessors[texCoordAccessorIndex].type);
            const ComponentType texCoordComponentType = static_cast<ComponentType>(_model.accessors[texCoordAccessorIndex].componentType);
            texCoordNumberOfComponents = getNumberOfComponents(texCoordAccessorType);
            const size_t texCoordComponentTypeSize = getComponentTypeSize(texCoordComponentType);
            texCoords.reset(new float[texCoordsCount * texCoordNumberOfComponents]);
            float *texCoordData = reinterpret_cast<float*>(&_model.buffers[texCoordBufferIndex].data[texCoordAccessorOffset + texCoordOffset]);
            const size_t texCoordStride = _model.bufferViews[texCoordBufferViewIndex].byteStride / texCoordComponentTypeSize;

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
    std::unique_ptr<float[]> colors = nullptr;
    for (const auto &attrIndexPair: attributes) {
        if (attrIndexPair.first.starts_with(COLOR)) {
            const size_t colorAccessorIndex = attributes.at(attrIndexPair.first);
            const size_t colorCount = _model.accessors[colorAccessorIndex].count;
            const size_t colorAccessorOffset = _model.accessors[colorAccessorIndex].byteOffset;
            const size_t colorBufferViewIndex = _model.accessors[colorAccessorIndex].bufferView;
            const size_t colorBufferIndex = _model.bufferViews[colorBufferViewIndex].buffer;
            const size_t length = _model.bufferViews[colorBufferViewIndex].byteLength;
            const size_t colorOffset = _model.bufferViews[colorBufferViewIndex].byteOffset;
            const AccessorType colorAccessorType = static_cast<AccessorType>(_model.accessors[colorAccessorIndex].type);
            const ComponentType colorComponentType = static_cast<ComponentType>(_model.accessors[colorAccessorIndex].componentType);
            colorNumberOfComponents = getNumberOfComponents(colorAccessorType);
            const size_t colorComponentTypeSize = getComponentTypeSize(colorComponentType);
            colors.reset(new float[colorCount * colorNumberOfComponents]);
            float *colorData = reinterpret_cast<float*>(&_model.buffers[colorBufferIndex].data[colorAccessorOffset + colorOffset]);
            const size_t colorStride = _model.bufferViews[colorBufferViewIndex].byteStride / colorComponentTypeSize;

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
    std::vector<Vertex> vertices(positionsCount);
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

    return vertices;
}

tinygltf::Primitive* SceneManager::findPrimitive(const int32_t primitiveIndex) {
    assert(primitiveIndex < _primitivesCount && "Invalid primitive index.");

    int32_t foundPrimitiveIndex = 0;
    tinygltf::Primitive *foundPrimitive = nullptr;

    if (0 == primitiveIndex) {
        foundPrimitive = &(*_model.meshes.begin()->primitives.begin());
    } else {
        int foundMeshIndex = 0;
        for (tinygltf::Mesh &mesh: _model.meshes) {
            bool primitiveFound = false;
            for (tinygltf::Primitive &primitive: mesh.primitives) {
                if (foundPrimitiveIndex < primitiveIndex) {
                    foundPrimitiveIndex++;
                } else {
                    foundPrimitive = &primitive;
                    primitiveFound = true;
                    break;
                }
            }

            if (primitiveFound)
                break;

            foundMeshIndex++;
        }

        int foundNodeIndex = 0;
        for(const tinygltf::Node &node: _model.nodes) {
            if (node.mesh == foundMeshIndex) {
                break;
            }
            foundNodeIndex++;
        }

        //_modelMatrix = _nodeMatrices[foundNodeIndex];
    }

    return foundPrimitive;
}


avocado::vulkan::Buffer SceneManager::formVertexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device, const int32_t primitiveIndex) {
    if (nullptr == _defaultScene)
        return avocado::vulkan::Buffer(0, usage, sharingMode, device);

    const tinygltf::Primitive * const targetPrimitive = findPrimitive(primitiveIndex);
    assert(targetPrimitive != nullptr && "No primitive found by index.");

    const std::vector<Vertex> totalVertices = copyVerticesFromPrimitive(*targetPrimitive);
    const size_t sizeBytes = sizeof(decltype(totalVertices)::value_type) * totalVertices.size();
    avocado::vulkan::Buffer buffer(sizeBytes, usage, sharingMode, device);
    buffer.allocateMemory(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    buffer.fill(totalVertices.data());
    buffer.bindMemory();
    return buffer;
}

int32_t SceneManager::getMaterialIndex(const int32_t primitiveIndex) {
    const tinygltf::Primitive * const targetPrimitive = findPrimitive(primitiveIndex);
    assert(targetPrimitive != nullptr && "No primitive found by index.");
    return targetPrimitive->material;
}

VkPrimitiveTopology SceneManager::getPrimitiveTopology(const int32_t primitiveIndex) noexcept {
    const tinygltf::Primitive * const targetPrimitive = findPrimitive(primitiveIndex);
    assert(targetPrimitive != nullptr && "No primitive found by index.");

    switch (targetPrimitive->mode) {
        case TINYGLTF_MODE_POINTS:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case TINYGLTF_MODE_LINE:
        case TINYGLTF_MODE_LINE_LOOP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case TINYGLTF_MODE_LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case TINYGLTF_MODE_TRIANGLES:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case TINYGLTF_MODE_TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case TINYGLTF_MODE_TRIANGLE_FAN:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    }

    assert(false && "Invalid input topology");
    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}


VkCullModeFlags SceneManager::getCullMode(const int32_t primitiveIndex) noexcept {
    const tinygltf::Primitive * const targetPrimitive = findPrimitive(primitiveIndex);
    assert(targetPrimitive != nullptr && "No primitive found by index.");

    if (_model.materials[targetPrimitive->material].doubleSided)
        return VK_CULL_MODE_FRONT_BIT;

    return VK_CULL_MODE_BACK_BIT;
}

std::vector<uint32_t> SceneManager::copyIndiciesFromPrimitive(const tinygltf::Primitive &primitive) {
    const size_t indicesAccessorIndex = primitive.indices;
    const size_t indicesBufferViewIndex = _model.accessors[indicesAccessorIndex].bufferView;
    const size_t indicesAccessorOffset = _model.accessors[indicesAccessorIndex].byteOffset;
    const size_t indicesBufferIndex = _model.bufferViews[indicesBufferViewIndex].buffer;
    const size_t indicesOffset = _model.bufferViews[indicesBufferViewIndex].byteOffset;
    const AccessorType indicesAccessorType = static_cast<AccessorType>(_model.accessors[indicesAccessorIndex].type);
    const ComponentType indicesComponentType = static_cast<ComponentType>(_model.accessors[indicesAccessorIndex].componentType);
    const size_t indicesNumberOfComponents = getNumberOfComponents(indicesAccessorType);
    const size_t indicesComponentTypeSize = getComponentTypeSize(indicesComponentType);
    const size_t indicesStride = _model.bufferViews[indicesBufferViewIndex].byteStride / indicesComponentTypeSize;
    unsigned char *indicesData = &_model.buffers[indicesBufferIndex].data[indicesAccessorOffset + indicesOffset];

    const size_t indicesCount = _model.accessors[indicesAccessorIndex].count;
    std::vector<uint32_t> indices(indicesCount);
    for (size_t i = 0, j = 0; i < indicesCount;) {
        for (size_t k = 0; k < indicesNumberOfComponents; ++k)
            memcpy(indices.data() + i + k, indicesData + (indicesComponentTypeSize * (j + k)), indicesComponentTypeSize);
        i += indicesNumberOfComponents;
        if (indicesStride > 0)
            j += indicesStride;
        else
            j++;
    }

    return indices;
}

avocado::vulkan::Buffer SceneManager::formIndexBuffer(avocado::vulkan::PhysicalDevice &physicalDevice, const VkBufferUsageFlagBits usage, const VkSharingMode sharingMode, avocado::vulkan::LogicalDevice &device, const int32_t primitiveIndex) {
    if (nullptr == _defaultScene)
        return avocado::vulkan::Buffer(0, usage, sharingMode, device);

    const tinygltf::Primitive * const targetPrimitive = findPrimitive(primitiveIndex);
    assert(targetPrimitive != nullptr && "No primitive found by index.");

    std::vector<uint32_t> totalIndices = copyIndiciesFromPrimitive(*targetPrimitive);
    const size_t sizeBytes = sizeof(decltype(totalIndices)::value_type) * totalIndices.size();
    avocado::vulkan::Buffer buffer(sizeBytes, usage, sharingMode, device);
    buffer.allocateMemory(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    buffer.fill(totalIndices.data());
    buffer.bindMemory();
    return buffer;
}

void SceneManager::parseNodes() {
    for (const tinygltf::Node &node: _model.nodes) {
        Node newNode;
        newNode._name = node.name;
        newNode._cameraIndex = node.camera;
        newNode._hasMesh = (node.mesh > -1);

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

        nodes.push_back(std::move(newNode));
    }

    for (size_t i = 0; i < _model.nodes.size(); ++i) {
        for (const int childIndex: _model.nodes[i].children)
            nodes[i].addChild(&nodes[childIndex]);
    }
    _nodesAreParsed = true;
}

void SceneManager::parseScenes() {
    assert(_nodesAreParsed);

    for (tinygltf::Scene &scene: _model.scenes) {
        Scene newScene;
        newScene._name = scene.name;
        for (const int nodeIndex: scene.nodes) {
            assert(nodeIndex < nodes.size());
            newScene._rootNodes.push_back(&nodes[nodeIndex]);
        }
        _scenes.push_back(std::move(newScene));
    }

    assert(_model.defaultScene < _scenes.size());
    _defaultScene = &_scenes[_model.defaultScene];
}

void SceneManager::parseCameras() {
    const tinygltf::Node * cameraNode = nullptr;
    const tinygltf::Camera *camera = nullptr;
    for (const tinygltf::Node &node: _model.nodes) {
        if (node.camera > -1) {
            cameraNode = &node;
            camera = &_model.cameras[node.camera];
            break;
        }
    }

    if (nullptr == camera) {
        LOG_INFORMATION("No camera in a model.");
        return;
    }

    _cameras.push_back(Camera {
        .perspective = math::perspectiveProjection(
            static_cast<float>(math::toDegrees(camera->perspective.yfov)),
            static_cast<float>(camera->perspective.aspectRatio),
            static_cast<float>(camera->perspective.znear),
            static_cast<float>(camera->perspective.zfar)),
        .position = math::vec3f{
            static_cast<float>(cameraNode->translation[0]),
            static_cast<float>(cameraNode->translation[1]),
            static_cast<float>(cameraNode->translation[2])},
        .up = math::vec3f{0.f, 1.f, 0.f}});

    math::vec3f forward(0.f, 0.f, -1.f);
    Camera &addedCamera = _cameras.back();
    if (cameraNode->matrix.empty()) {
        const std::vector<double> scale = cameraNode->scale.empty() ?
            std::vector<double>{1., 1., 1.} : cameraNode->scale;
        const std::vector<double> rotation = cameraNode->rotation.empty() ?
            std::vector<double>{0., 0., 0., 1.} : cameraNode->rotation;
        const std::vector<double> translation = cameraNode->translation.empty() ?
            std::vector<double>{0., 0., 0., 0.} : cameraNode->translation;
        math::Quaternion rotationQuat = math::Quaternion::fromTinyGltf(rotation);
        rotationQuat.normalize();
        addedCamera.localMatrix = math::Mat4x4::fromSRT(scale,
            {rotationQuat.x,rotationQuat.y,rotationQuat.z,rotationQuat.w},
            translation);
        addedCamera.up = rotationQuat.rotateVector(addedCamera.up);
        forward = rotationQuat.rotateVector(forward);
    } else {
        addedCamera.localMatrix = math::Mat4x4::fromTinyGltf(cameraNode->matrix);
    }

    addedCamera.targetPosition = addedCamera.position + forward;
}

}
