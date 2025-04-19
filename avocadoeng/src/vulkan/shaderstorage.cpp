#include "shaderstorage.hpp"

#include "logicaldevice.hpp"
#include "structuretypes.hpp"

#include "../utils.hpp"

#include <filesystem>
#include <string>

namespace avocado::vulkan {

ShaderStorage::ShaderStorage(LogicalDevice &device):
    _device(device) {}

std::vector<VkPipelineShaderStageCreateInfo> ShaderStorage::createStageCIs(const std::initializer_list<ShaderId> &shaderIds) {
    std::vector<VkPipelineShaderStageCreateInfo> result;
    for (const ShaderId shaderId: shaderIds) {
        if (_shaderModulesVertex.contains(shaderId)) {
            const ShaderModulePtr &modulePtr = _shaderModulesVertex.at(shaderId);
            result.push_back(createStageCI(modulePtr, VK_SHADER_STAGE_VERTEX_BIT));
        } else if (_shaderModulesFragment.contains(shaderId)) {
            const ShaderModulePtr &modulePtr = _shaderModulesFragment.at(shaderId);
            result.push_back(createStageCI(modulePtr, VK_SHADER_STAGE_FRAGMENT_BIT));
        }
    }

    return result;
}

void ShaderStorage::loadShaders(const std::string &shaderPath) {
    loadShader(shaderPath + "/process_input.vert.spv", ShaderIdConst::VERTEX_PROCESS_INPUT);
    loadShader(shaderPath + "/red_color.frag.spv", ShaderIdConst::FRAGMENT_RED_COLOR);
    loadShader(shaderPath + "/one_texture.frag.spv", ShaderIdConst::FRAGMENT_ONE_TEXTURE);
}

void ShaderStorage::loadShader(const std::string &shaderPath, const ShaderId id) {
    if (std::filesystem::is_regular_file(shaderPath)) {
        const std::vector<char> &fileContent = avocado::utils::readFile(shaderPath);
        if (avocado::utils::endsWith(shaderPath, ".vert.spv"))
            addShaderModule(fileContent, VK_SHADER_STAGE_VERTEX_BIT, id);
        else if (avocado::utils::endsWith(shaderPath, ".frag.spv"))
            addShaderModule(fileContent, VK_SHADER_STAGE_FRAGMENT_BIT, id);
    }
}

VkPipelineShaderStageCreateInfo ShaderStorage::createStageCI(const ShaderModulePtr &shaderModulePtr, const VkShaderStageFlagBits shaderType) {
    assert(nullptr != shaderModulePtr);

    DEFINE_VK_STRUCTURE(VkPipelineShaderStageCreateInfo, createInfo);
    createInfo.stage = shaderType;
    createInfo.module = shaderModulePtr.get();
    createInfo.pName = "main"; // Entry point.
    return createInfo;
}

void ShaderStorage::addShaderModule(const std::vector<char> &data, const VkShaderStageFlagBits shType, const ShaderId id) {
    if (_shaderModulesFragment.contains(id) || _shaderModulesVertex.contains(id)) {
        assert("There's already a shader module with specified id.");
        return;
    }

    DEFINE_VK_STRUCTURE(VkShaderModuleCreateInfo, createInfo);
    createInfo.codeSize = data.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    const VkResult result = vkCreateShaderModule(_device.getHandle(), &createInfo, nullptr, &shaderModule);
    if (VK_SUCCESS == result) {
        if (VK_SHADER_STAGE_VERTEX_BIT == shType)
            _shaderModulesVertex.emplace(id, makeObjectPtr<VkShaderModule>(_device, shaderModule));
        else if (VK_SHADER_STAGE_FRAGMENT_BIT == shType)
            _shaderModulesFragment.emplace(id, makeObjectPtr<VkShaderModule>(_device, shaderModule));
    }
}

} // namespace avocado::vulkan

