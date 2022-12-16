#ifndef AVOCADO_VULKAN_VERTEX_INPUT_STATE
#define AVOCADO_VULKAN_VERTEX_INPUT_STATE

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class VertexInputState {
public:
    void addBindingDescription(const uint32_t binding, const uint32_t stride, VkVertexInputRate inRate);
    void addAttributeDescription(const uint32_t loc, const uint32_t binding, const VkFormat format, const uint32_t offset);

    VkVertexInputBindingDescription* getBindingDescriptionData() noexcept;
    VkVertexInputAttributeDescription* getAttributeDescriptionData() noexcept;
    uint32_t getBindingDescriptionsCount() const noexcept;
    uint32_t getAttributeDescriptionsCount() const noexcept;

    VkPipelineVertexInputStateCreateInfo createCreateInfo() noexcept;

private:
    std::vector<VkVertexInputBindingDescription> _bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;
};

}

#endif

