#ifndef AVOCADO_VULKAN_DYNAMIC_STATE
#define AVOCADO_VULKAN_DYNAMIC_STATE

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class DynamicState {
public:
    DynamicState() = default;
    explicit DynamicState(const std::vector<VkDynamicState> &dynStates);
    explicit DynamicState(std::vector<VkDynamicState> &&dynStates);

    uint32_t getDynamicStateCount() const noexcept;
    const VkDynamicState* getDynamicStates() noexcept;
    void setDynamicStates(const std::vector<VkDynamicState> &dynStates);
    void setDynamicStates(std::vector<VkDynamicState> &&dynStates);

    VkPipelineDynamicStateCreateInfo createCreateInfo();

private:
    std::vector<VkDynamicState> _dynamicStates;
};

}

#endif

