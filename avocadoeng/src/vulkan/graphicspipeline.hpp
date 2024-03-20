#ifndef AVOCADO_VULKAN_GRAPHICS_PIPELINE
#define AVOCADO_VULKAN_GRAPHICS_PIPELINE

#include "../errorstorage.hpp"

#include "logicaldevice.hpp"
#include "pointertypes.hpp"
#include "vulkan_core.h"
#include "states/colorblendstate.hpp"
#include "states/dynamicstate.hpp"
#include "states/inputasmstate.hpp"
#include "states/multisamplestate.hpp"
#include "states/rasterizationstate.hpp"
#include "states/vertexinputstate.hpp"
#include "states/viewportstate.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <vector>

namespace avocado::vulkan {

class ColorBlendState;
class DynamicState;
class InputAsmState;
class MultisampleState;
class RasterizationState;
class VertexInputState;
class ViewportState;

class GraphicsPipelineBuilder: public avocado::core::ErrorStorage {
public:
    explicit GraphicsPipelineBuilder(LogicalDevice &device);

    VkPipelineLayout getPipelineLayout() noexcept;
    void setColorBlendState(std::unique_ptr<ColorBlendState> state) noexcept;
    void setDynamicState(std::unique_ptr<DynamicState> dynState) noexcept;
    void setInputAsmState(std::unique_ptr<InputAsmState> inAsmState) noexcept;
    void setMultisampleState(std::unique_ptr<MultisampleState> multisampleState) noexcept;
    void setRasterizationState(std::unique_ptr<RasterizationState> rasterizationState) noexcept;
    void setVertexInputState(std::unique_ptr<VertexInputState> vertexInState) noexcept;
    void setViewportState(std::unique_ptr<ViewportState> viewportState) noexcept;
    void addFragmentShaderModules(const std::vector<std::vector<char>> &shaderModules);
    void addVertexShaderModules(const std::vector<std::vector<char>> &shaderModules);
    void setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> &layouts);

private:
    template <typename T>
    inline auto createStateCreateInfo(T &state) {
        // todo check if T has createCreateInfo() function and do static_assert.
        return state.createCreateInfo();
    }

    VkPipelineShaderStageCreateInfo addShaderModule(const std::vector<char> &data, const VkShaderStageFlagBits shType);

    LogicalDevice &_logicalDevice;
    std::vector<ShaderModulePtr> _shaderModules;

    std::vector<VkPipelineShaderStageCreateInfo> _shaderModuleCIs;
    std::unique_ptr<ColorBlendState> _colorBlendState = nullptr;
    std::unique_ptr<DynamicState> _dynamicState = nullptr;
    std::unique_ptr<VertexInputState> _vertexInputState = nullptr;
    std::unique_ptr<InputAsmState> _inputAsmState = nullptr;
    std::unique_ptr<MultisampleState> _multisampleState = nullptr;
    std::unique_ptr<RasterizationState> _rasterizationState = nullptr;
    std::unique_ptr<ViewportState> _viewportState = nullptr;
    avocado::vulkan::ObjectPtr<VkPipelineLayout> _pipelineLayout;
    std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;

public:
    PipelinePtr buildPipeline(VkRenderPass renderPass);
    void destroyPipeline();
};

} // namespace avocado::vulkan.

#endif

