#ifndef AVOCADO_VULKAN_VIEWPORT_STATE
#define AVOCADO_VULKAN_VIEWPORT_STATE

#include <vulkan/vulkan_core.h>

#include <vector>

namespace avocado::vulkan {

class ViewportState {
public:
    ViewportState() = default;
    explicit ViewportState(const std::vector<VkViewport> &viewports, const std::vector<VkRect2D> &scissors);
    explicit ViewportState(std::vector<VkViewport> &&viewports, std::vector<VkRect2D> &&scissors);

    uint32_t getViewportCount() const noexcept;
    const VkViewport* getViewports() const noexcept;
    void setViewports(const std::vector<VkViewport> &viewports);
    void setViewPorts(std::vector<VkViewport> &&viewports);

    uint32_t getScissorCount() const noexcept;
    const VkRect2D* getScissors() const noexcept;
    void setScissors(const std::vector<VkRect2D> &scissors);
    void setScissors(std::vector<VkRect2D> &&scissors);

    VkPipelineViewportStateCreateInfo createCreateInfo();

private:
    std::vector<VkViewport> _viewports;
    std::vector<VkRect2D> _scissors;
};

}

#endif

