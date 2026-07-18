#ifndef AVOCADO_VULKAN_POINTERTYPES_HPP
#define AVOCADO_VULKAN_POINTERTYPES_HPP

#include "objectdeleter.hpp"

#define DECLARE_POINTER_TYPES(Type)\
    using Type##Ptr = ObjectPtr<Vk##Type>;\
    using Type##SharedPtr = ObjectSharedPtr<Vk##Type>;
#define DECLARE_POINTER_FUNDAMENTAL_TYPE(Type) using Type##Ptr = FundamentalObjectPtr<Vk##Type>;
#define DECLARE_POINTER_ALLOCATED_TYPE(Type) using Type##Ptr = AllocatedObjectPtr<Vk##Type>;

namespace avocado::vulkan
{

DECLARE_POINTER_TYPES(CommandPool)
DECLARE_POINTER_TYPES(DescriptorPool)
DECLARE_POINTER_TYPES(DescriptorSetLayout)
DECLARE_POINTER_FUNDAMENTAL_TYPE(Device)
DECLARE_POINTER_ALLOCATED_TYPE(DeviceMemory)
DECLARE_POINTER_TYPES(Fence)
DECLARE_POINTER_TYPES(Image)
DECLARE_POINTER_TYPES(ImageView)
DECLARE_POINTER_FUNDAMENTAL_TYPE(Instance)
DECLARE_POINTER_TYPES(PhysicalDevice)
DECLARE_POINTER_TYPES(Pipeline)
DECLARE_POINTER_TYPES(PipelineLayout)
DECLARE_POINTER_TYPES(RenderPass)
DECLARE_POINTER_TYPES(Sampler)
DECLARE_POINTER_TYPES(Semaphore)
DECLARE_POINTER_TYPES(ShaderModule)
DECLARE_POINTER_TYPES(SwapchainKHR)

} // namespace avocado::vulkan.

#endif // AVOCADO_VULKAN_POINTERTYPES_HPP
