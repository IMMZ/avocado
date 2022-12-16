#include "viewportstate.hpp"

#include "../vkutils.hpp"

namespace avocado::vulkan {

ViewportState::ViewportState(const std::vector<VkViewport> &viewports, const std::vector<VkRect2D> &scissors):
    _viewports(viewports),
    _scissors(scissors) {
}

ViewportState::ViewportState(std::vector<VkViewport> &&viewports, std::vector<VkRect2D> &&scissors):
    _viewports(std::move(viewports)),
    _scissors(std::move(scissors)) {

}

uint32_t ViewportState::getViewportCount() const noexcept {
    return static_cast<uint32_t>(_viewports.size());
}

const VkViewport* ViewportState::getViewports() const noexcept {
    return _viewports.data();
}

void ViewportState::setViewports(const std::vector<VkViewport> &viewports) {
    _viewports = viewports;
}

void ViewportState::setViewPorts(std::vector<VkViewport> &&viewports) noexcept {
    _viewports = std::move(viewports);
}

uint32_t ViewportState::getScissorCount() const noexcept {
    return static_cast<uint32_t>(_scissors.size());
}

const VkRect2D* ViewportState::getScissors() const noexcept {
    return _scissors.data();
}

void ViewportState::setScissors(const std::vector<VkRect2D> &scissors) {
    _scissors = scissors;
}

void ViewportState::setScissors(std::vector<VkRect2D> &&scissors) noexcept {
    _scissors = std::move(scissors);
}

VkPipelineViewportStateCreateInfo ViewportState::createCreateInfo() noexcept {
    auto viewportStateCI = createStruct<VkPipelineViewportStateCreateInfo>();
    viewportStateCI.viewportCount = getViewportCount();
    if (getViewportCount() > 0)
        viewportStateCI.pViewports = getViewports();

    viewportStateCI.scissorCount = getScissorCount();
    if (getScissorCount() > 0)
        viewportStateCI.pScissors = getScissors();
    return viewportStateCI;
}

}
