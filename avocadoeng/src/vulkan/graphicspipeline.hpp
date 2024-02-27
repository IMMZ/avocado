#ifndef AVOCADO_VULKAN_GRAPHICS_PIPELINE
#define AVOCADO_VULKAN_GRAPHICS_PIPELINE

#include "../errorstorage.hpp"

#include "logicaldevice.hpp"
#include "vulkan_core.h"

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
    VkDescriptorSetLayout getDescriptorSetLayout() noexcept;
    void setColorBlendState(ColorBlendState &state) noexcept;
    void setDynamicState(DynamicState &dynState) noexcept;
    void setInputAsmState(InputAsmState &inAsmState) noexcept;
    void setMultisampleState(MultisampleState &multisampleState) noexcept;
    void setRasterizationState(RasterizationState &rasterizationState) noexcept;
    void setVertexInputState(VertexInputState &vertexInState) noexcept;
    void setViewportState(ViewportState &viewportState) noexcept;
    void addFragmentShaderModules(const std::vector<std::vector<char>> &shaderModules);
    void addVertexShaderModules(const std::vector<std::vector<char>> &shaderModules);

private:
    template <typename T>
    inline auto createStateCreateInfo(T &state) {
        // todo check if T has createCreateInfo() function and do static_assert.
        return state.createCreateInfo();
    }

    VkPipelineShaderStageCreateInfo addShaderModule(const std::vector<char> &data, const VkShaderStageFlagBits shType);

    LogicalDevice &_logicalDevice;
    using ShaderModuleDeleter = decltype(std::bind(vkDestroyShaderModule, _logicalDevice.getHandle(), std::placeholders::_1, nullptr));
    std::vector<std::unique_ptr<VkShaderModule_T, ShaderModuleDeleter>> _shaderModules;

    std::vector<VkPipelineShaderStageCreateInfo> _shaderModuleCIs;
    ColorBlendState *_colorBlendState = nullptr;
    DynamicState *_dynamicState = nullptr;
    VertexInputState *_vertexInputState = nullptr;
    InputAsmState *_inputAsmState = nullptr;
    MultisampleState *_multisampleState = nullptr;
    RasterizationState *_rasterizationState = nullptr;
    ViewportState *_viewportState = nullptr;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;

public:
    // todo maybe replace it by handle + dtor? And rename class to GraphicsPipeline?
    // We must take into account that this class could create many pipelines (in future, see todo in cpp).
    using PipelineDestroyer = decltype(std::bind(vkDestroyPipeline, _logicalDevice.getHandle(), std::placeholders::_1, nullptr));
    using PipelineUniquePtr = std::unique_ptr<std::remove_pointer_t<VkPipeline>, PipelineDestroyer>;
    PipelineUniquePtr buildPipeline(VkRenderPass renderPass);
    void destroyPipeline();
};

} // namespace avocado::vulkan.

#endif

