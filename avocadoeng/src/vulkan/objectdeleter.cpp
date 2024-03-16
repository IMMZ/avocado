#include "objectdeleter.hpp"

#include "logicaldevice.hpp"
#include "vulkan_core.h"

#define DEFINE_SPECIALIZATION(ObjectName) template<> void destroyObject<Vk##ObjectName>(LogicalDevice &logicalDevice, Vk##ObjectName objHandle) {\
    vkDestroy##ObjectName(logicalDevice.getHandle(), objHandle, nullptr);\
}\

namespace avocado::vulkan::internal {

DEFINE_SPECIALIZATION(CommandPool)
DEFINE_SPECIALIZATION(DescriptorPool)
DEFINE_SPECIALIZATION(DescriptorSetLayout)
DEFINE_SPECIALIZATION(Fence)
DEFINE_SPECIALIZATION(Image)
DEFINE_SPECIALIZATION(ImageView)
DEFINE_SPECIALIZATION(PipelineLayout)
DEFINE_SPECIALIZATION(Semaphore)
DEFINE_SPECIALIZATION(Sampler)

} // namespace avocado::vulkan::internal.
