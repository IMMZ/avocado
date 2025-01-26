#ifndef AVOCADO_VULKAN_SHADER_STORAGE
#define AVOCADO_VULKAN_SHADER_STORAGE

#include "pointertypes.hpp"

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace avocado::vulkan {

struct ShaderIdConst {
    static constexpr uint32_t VERTEX_PROCESS_INPUT = 0;
    static constexpr uint32_t FRAGMENT_RED_COLOR = 1;
    static constexpr uint32_t FRAGMENT_ONE_TEXTURE = 2;
};

class ShaderStorage {
public:
    explicit ShaderStorage(LogicalDevice &device);

    using ShaderId = uint32_t;
    std::vector<VkPipelineShaderStageCreateInfo> createStageCIs(const std::initializer_list<ShaderId> &shaderIds);
    void loadShaders(const std::string &shaderPath);

private:
    VkPipelineShaderStageCreateInfo createStageCI(const ShaderModulePtr &shaderModulePtr, const VkShaderStageFlagBits shaderType);
    void loadShader(const std::string &shaderPath, const ShaderId id);
    void addShaderModule(const std::vector<char> &data, const VkShaderStageFlagBits shType, const ShaderId id);

    std::unordered_map<ShaderId, ShaderModulePtr> _shaderModulesVertex;
    std::unordered_map<ShaderId, ShaderModulePtr> _shaderModulesFragment;

    LogicalDevice &_device;
};

} // namespace avocado::vulkan.

#endif // ifndef AVOCADO_VULKAN_SHADER_STORAGE
