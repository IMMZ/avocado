#include "rasterizationstate.hpp"

#include "../vkutils.hpp"

namespace avocado::vulkan {

VkBool32 RasterizationState::isDepthClampEnabled() const noexcept {
    return _depthClampEnable;
}

void RasterizationState::setDepthClampEnabled(const VkBool32 enable) {
    _depthClampEnable = enable;
}

VkBool32 RasterizationState::isRasterizerDiscardEnabled() const noexcept {
    return _rasterizerDiscardEnable;
}

void RasterizationState::setRasterizerDiscardEnabled(const VkBool32 enable) {
    _rasterizerDiscardEnable = enable;
}

VkBool32 RasterizationState::isDepthBiasEnabled() const noexcept {
    return _depthBiasEnable;
}

void RasterizationState::setDepthBiasEnabled(const VkBool32 enable) {
    _depthBiasEnable = enable;
}

float RasterizationState::getLineWidth() const noexcept {
    return _lineWidth;
}

void RasterizationState::setLineWidth(const float lineWidth) {
    _lineWidth = lineWidth;
}

VkPolygonMode RasterizationState::getPolygonMode() const noexcept {
    return _polygonMode;
}

void RasterizationState::setPolygonMode(const VkPolygonMode polygonMode) {
    _polygonMode = polygonMode;
}

VkFlags RasterizationState::getCullMode() const noexcept {
    return _cullMode;
}

void RasterizationState::setCullMode(const VkFlags cullMode) {
    _cullMode = cullMode;
}

VkFrontFace RasterizationState::getFrontFace() const noexcept {
    return _frontFace;
}

void RasterizationState::setFrontFace(const VkFrontFace frontFace) {
    _frontFace = frontFace;
}

VkPipelineRasterizationStateCreateInfo RasterizationState::createCreateInfo() {
    auto rasterizationCI = createStruct<VkPipelineRasterizationStateCreateInfo>();
    rasterizationCI.depthClampEnable = isDepthClampEnabled();
    rasterizationCI.rasterizerDiscardEnable = isRasterizerDiscardEnabled();
    rasterizationCI.polygonMode = getPolygonMode();
    rasterizationCI.lineWidth = getLineWidth();
    rasterizationCI.cullMode = getCullMode();
    rasterizationCI.frontFace = getFrontFace();
    rasterizationCI.depthBiasEnable = isDepthBiasEnabled();
    return rasterizationCI;
}

}
