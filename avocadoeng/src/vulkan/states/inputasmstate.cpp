#include "inputasmstate.hpp"

#include "../vkutils.hpp"

namespace avocado::vulkan {

InputAsmState::InputAsmState(VkPrimitiveTopology topology):
    _topology(topology) {
}

VkPrimitiveTopology InputAsmState::getTopology() const noexcept {
    return _topology;
}

VkPipelineInputAssemblyStateCreateInfo InputAsmState::createCreateInfo() noexcept {
    auto inputAsmStateCI = createStruct<VkPipelineInputAssemblyStateCreateInfo>();
    inputAsmStateCI.topology = getTopology();
    return inputAsmStateCI;
}

}

