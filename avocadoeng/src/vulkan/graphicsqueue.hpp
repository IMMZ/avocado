#ifndef AVOCADO_VULKAN_GRAPHICS_QUEUE
#define AVOCADO_VULKAN_GRAPHICS_QUEUE

#include "queue.hpp"

namespace avocado::vulkan {

class GraphicsQueue final: public Queue {
public:
    explicit GraphicsQueue(VkQueue queue);
    
    void setSemaphores(const std::vector<VkSemaphore> &waitSemaphores, const std::vector<VkSemaphore> &signalSemaphores);
    void setCommandBuffers(std::vector<CommandBuffer> &commandBuffers);
    
    enum class PipelineStageFlag: uint32_t {
        TopOfPipe = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
        , DrawIndirect = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
        , VertexInput = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
        , VertexShader = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        , TessellationControlShader = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
        , TessellationEvaluationShader = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
        , GeometryShader = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
        , FragmentShader = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        , EarlyFragmentTests = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        , LateFragmentTests = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
        , ColorAttachmentOutput = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        , ComputeShader = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
        , Transfer = VK_PIPELINE_STAGE_TRANSFER_BIT
        , BottomOfPipe = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
        , Host = VK_PIPELINE_STAGE_HOST_BIT
        , AllGraphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT
        , AllCommands = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
        , None = VK_PIPELINE_STAGE_NONE
        , TransformFeedbackExt = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
        , ConditionalRenderingExt = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT
        , AccelerationStructureBuildKHR = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
        , RayTracingShaderKHR = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR
        , TaskShader = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV
        , MeshShader = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV
        , FragmentDensityProcessExt = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT
        , FragmentShadingRateAttachmentKHR = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
        , CommandPreprocessNV = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV
        , ShadingRateImageNV = VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV
        , RayTracingShaderNV = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV
        , AccelerationStructureBuildNV = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV
        , NoneKHR = VK_PIPELINE_STAGE_NONE_KHR
    };
    void setPipelineStageFlags(const std::vector<PipelineStageFlag> &flags);

    void submit(VkFence fence);

private:
    std::vector<VkPipelineStageFlags> _pipelineStageFlags;
    VkSubmitInfo _submitInfo{};
};

}

#endif

