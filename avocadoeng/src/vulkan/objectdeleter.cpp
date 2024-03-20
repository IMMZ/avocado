#include "objectdeleter.hpp"

#include "logicaldevice.hpp"

#define DEFINE_SPECIALIZATION(ObjectName) template<> void destroyObject<Vk##ObjectName>(LogicalDevice &logicalDevice, Vk##ObjectName objHandle) {\
    vkDestroy##ObjectName(logicalDevice.getHandle(), objHandle, nullptr);\
}\

#define DEFINE_FUNDAMENTAL_SPECIALIZATION(ObjectName) template<> void destroyFundamentalObject<Vk##ObjectName>(Vk##ObjectName objHandle) {\
    vkDestroy##ObjectName(objHandle, nullptr);\
}\

namespace avocado::vulkan::internal {

DEFINE_FUNDAMENTAL_SPECIALIZATION(Device)
DEFINE_FUNDAMENTAL_SPECIALIZATION(Instance)

DEFINE_SPECIALIZATION(CommandPool)
DEFINE_SPECIALIZATION(DescriptorPool)
DEFINE_SPECIALIZATION(DescriptorSetLayout)
DEFINE_SPECIALIZATION(Fence)
DEFINE_SPECIALIZATION(Image)
DEFINE_SPECIALIZATION(ImageView)
DEFINE_SPECIALIZATION(Pipeline)
DEFINE_SPECIALIZATION(PipelineLayout)
DEFINE_SPECIALIZATION(RenderPass)
DEFINE_SPECIALIZATION(Sampler)
DEFINE_SPECIALIZATION(Semaphore)
DEFINE_SPECIALIZATION(ShaderModule)
DEFINE_SPECIALIZATION(SwapchainKHR)

template <>
void freeAllocation<VkDeviceMemory>(LogicalDevice &device, VkDeviceMemory allocatedObjectHandle) noexcept {
    vkFreeMemory(device.getHandle(), allocatedObjectHandle, nullptr);
}

} // namespace avocado::vulkan::internal.
