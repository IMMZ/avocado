#ifndef AVOCADO_VULKAN_BUFFER
#define AVOCADO_VULKAN_BUFFER

#include "../errorstorage.hpp"

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

class LogicalDevice;

class Buffer: public core::ErrorStorage {
public:
    enum class Usage: uint32_t {
        TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        , TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT
        , UniformTexel = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
        , StorageTexel = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
        , Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        , Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        , Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        , Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        , Indirect = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
        , ShaderDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
#ifdef VK_ENABLE_BETA_EXTENSIONS
        , VideoDecodeSrcKHR = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR
        , VideoDecodeDstKHR = VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR
        , VideoEncodeSrcKHR = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR
        , VideoEncodeDstKHR = VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR
#endif
        , TransformFeedbackEXT = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT
        , TransformFeedbackCounter = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT
        , ConditionalRendering = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT
        , AccelerationStructureBuildInputReadOnlyKHR = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
        , AccelerationStructureStorageKHR = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
        , ShaderBindingTableKHR = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
        , RayTracingNV = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV
        , ShaderDeviceAddressEXT = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT
        , ShaderDeviceAddressKHR = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR
    };

    enum class SharingMode {
        Exclusive = VK_SHARING_MODE_EXCLUSIVE
        , Concurrent = VK_SHARING_MODE_CONCURRENT
    };

    explicit Buffer(const VkDeviceSize size, const Usage usage, const SharingMode sharingMode, LogicalDevice &device);
    ~Buffer();

    VkBuffer getHandle();
    Usage getUsage() const;

private:
    VkDevice _dev = VK_NULL_HANDLE;
    VkBuffer _buf = VK_NULL_HANDLE;
    Usage _usage = Usage::TransferSrc;
};

}
#endif

