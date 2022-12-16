#ifndef AVOCADO_VULKAN_INPUT_ASM_STATE
#define AVOCADO_VULKAN_INPUT_ASM_STATE

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

class InputAsmState {
public:
    explicit InputAsmState(VkPrimitiveTopology topology);

    VkPrimitiveTopology getTopology() const noexcept;

    VkPipelineInputAssemblyStateCreateInfo createCreateInfo() noexcept;

private:
    VkPrimitiveTopology _topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
};

}
#endif

