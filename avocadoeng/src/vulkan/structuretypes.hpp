#ifndef AVOCADO_VULKAN_INTERNAL_STRUCTURE_TYPES
#define AVOCADO_VULKAN_INTERNAL_STRUCTURE_TYPES

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define DEFINE_STRUCTURE_TYPE(STRUCT, STRUCT_TYPE)\
template<>\
inline constexpr VkStructureType StructureType<Vk##STRUCT> = VK_STRUCTURE_TYPE_##STRUCT_TYPE

#define DEFINE_OBJECT_TYPE(OBJECT, OBJECT_TYPE)\
template <>\
inline constexpr VkObjectType ObjectType<Vk##OBJECT> = VK_OBJECT_TYPE_##OBJECT_TYPE

#define FILL_S_TYPE(variable) variable.sType = avocado::vulkan::StructureType<decltype(variable)>
#define FILL_OBJECT_TYPE(variable) variable.objectType = avocado::vulkan::ObjectType<decltype(variable)>

namespace avocado::vulkan {

// Object types.
template <typename T>
inline constexpr VkObjectType ObjectType = VK_OBJECT_TYPE_MAX_ENUM;

DEFINE_OBJECT_TYPE(CommandBuffer, COMMAND_BUFFER);
DEFINE_OBJECT_TYPE(SwapchainKHR, SWAPCHAIN_KHR);
DEFINE_OBJECT_TYPE(Queue, QUEUE);
DEFINE_OBJECT_TYPE(PipelineLayout, PIPELINE_LAYOUT);

// Structure types.
template<typename T>
inline constexpr VkStructureType StructureType = VK_STRUCTURE_TYPE_MAX_ENUM;

DEFINE_STRUCTURE_TYPE(ApplicationInfo, APPLICATION_INFO);
DEFINE_STRUCTURE_TYPE(BufferCreateInfo, BUFFER_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(CommandBufferAllocateInfo, COMMAND_BUFFER_ALLOCATE_INFO);
DEFINE_STRUCTURE_TYPE(CommandBufferBeginInfo, COMMAND_BUFFER_BEGIN_INFO);
DEFINE_STRUCTURE_TYPE(CommandPoolCreateInfo, COMMAND_POOL_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(DebugUtilsObjectNameInfoEXT, DEBUG_UTILS_OBJECT_NAME_INFO_EXT);
DEFINE_STRUCTURE_TYPE(DebugUtilsObjectTagInfoEXT, DEBUG_UTILS_OBJECT_TAG_INFO_EXT);
DEFINE_STRUCTURE_TYPE(DescriptorPoolCreateInfo, DESCRIPTOR_POOL_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(DescriptorSetAllocateInfo, DESCRIPTOR_SET_ALLOCATE_INFO);
DEFINE_STRUCTURE_TYPE(DescriptorSetLayoutCreateInfo, DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(DeviceCreateInfo, DEVICE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(DeviceQueueCreateInfo, DEVICE_QUEUE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(FenceCreateInfo, FENCE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(FramebufferCreateInfo, FRAMEBUFFER_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(GraphicsPipelineCreateInfo, GRAPHICS_PIPELINE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(ImageCreateInfo, IMAGE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(ImageViewCreateInfo, IMAGE_VIEW_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(InstanceCreateInfo, INSTANCE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(MemoryAllocateInfo, MEMORY_ALLOCATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineColorBlendStateCreateInfo, PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineDynamicStateCreateInfo, PIPELINE_DYNAMIC_STATE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineInputAssemblyStateCreateInfo, PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineLayoutCreateInfo, PIPELINE_LAYOUT_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineMultisampleStateCreateInfo, PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineRasterizationStateCreateInfo, PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineShaderStageCreateInfo, PIPELINE_SHADER_STAGE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineVertexInputStateCreateInfo, PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PipelineViewportStateCreateInfo, PIPELINE_VIEWPORT_STATE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(PresentInfoKHR, PRESENT_INFO_KHR);
DEFINE_STRUCTURE_TYPE(RenderPassBeginInfo, RENDER_PASS_BEGIN_INFO);
DEFINE_STRUCTURE_TYPE(RenderPassCreateInfo, RENDER_PASS_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(SemaphoreCreateInfo, SEMAPHORE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(ShaderModuleCreateInfo, SHADER_MODULE_CREATE_INFO);
DEFINE_STRUCTURE_TYPE(SwapchainCreateInfoKHR, SWAPCHAIN_CREATE_INFO_KHR);
DEFINE_STRUCTURE_TYPE(SubmitInfo, SUBMIT_INFO);
DEFINE_STRUCTURE_TYPE(WriteDescriptorSet, WRITE_DESCRIPTOR_SET);

}

#undef DEFINE_STRUCTURE_TYPE
#undef DEFINE_OBJECT_TYPE

#endif
