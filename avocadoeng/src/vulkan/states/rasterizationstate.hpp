#ifndef AVOCADO_VULKAN_RASTERIZATION_STATE
#define AVOCADO_VULKAN_RASTERIZATION_STATE

#include <vulkan/vulkan_core.h>

namespace avocado::vulkan {

class RasterizationState {
public:
    VkBool32 isDepthClampEnabled() const noexcept;
    void setDepthClampEnabled(const VkBool32 enable) noexcept;
    VkBool32 isRasterizerDiscardEnabled() const noexcept;
    void setRasterizerDiscardEnabled(const VkBool32 enable) noexcept;
    VkBool32 isDepthBiasEnabled() const noexcept;
    void setDepthBiasEnabled(const VkBool32 enable) noexcept;
    float getLineWidth() const noexcept;
    void setLineWidth(const float lineWidth) noexcept;
    VkPolygonMode getPolygonMode() const noexcept;
    void setPolygonMode(const VkPolygonMode polygonMode) noexcept;
    VkFlags getCullMode() const noexcept;
    void setCullMode(const VkFlags cullMode) noexcept;
    VkFrontFace getFrontFace() const noexcept;
    void setFrontFace(const VkFrontFace frontFace) noexcept;

    VkPipelineRasterizationStateCreateInfo createCreateInfo() noexcept;

private:
    VkBool32 _depthClampEnable = VK_FALSE, _rasterizerDiscardEnable = VK_FALSE, _depthBiasEnable = VK_FALSE;
    float _lineWidth = 1.f;
    VkPolygonMode _polygonMode = VK_POLYGON_MODE_MAX_ENUM;
    VkFlags _cullMode = VK_CULL_MODE_NONE;
    VkFrontFace _frontFace = VK_FRONT_FACE_MAX_ENUM;
};

}

#endif

