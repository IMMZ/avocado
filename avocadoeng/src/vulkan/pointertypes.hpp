#ifndef AVOCADO_VULKAN_POINTERTYPES_HPP
#define AVOCADO_VULKAN_POINTERTYPES_HPP

#include "objectdeleter.hpp"

#define DECLARE_POINTER_TYPE(Type) using Type##Ptr = ObjectPtr<Vk##Type>;
#define DECLARE_POINTER_FUNDAMENTAL_TYPE(Type) using Type##Ptr = FundamentalObjectPtr<Vk##Type>;

namespace avocado::vulkan
{

DECLARE_POINTER_TYPE(CommandPool)
DECLARE_POINTER_TYPE(DescriptorPool)
DECLARE_POINTER_TYPE(DescriptorSetLayout)
DECLARE_POINTER_FUNDAMENTAL_TYPE(Device)
DECLARE_POINTER_TYPE(Fence)
DECLARE_POINTER_TYPE(ImageView)
DECLARE_POINTER_FUNDAMENTAL_TYPE(Instance)
DECLARE_POINTER_TYPE(PhysicalDevice)
DECLARE_POINTER_TYPE(Pipeline)
DECLARE_POINTER_TYPE(RenderPass)
DECLARE_POINTER_TYPE(Sampler)
DECLARE_POINTER_TYPE(Semaphore)
DECLARE_POINTER_TYPE(ShaderModule)
DECLARE_POINTER_TYPE(SwapchainKHR)

} // namespace avocado::vulkan.

#endif // AVOCADO_VULKAN_POINTERTYPES_HPP
