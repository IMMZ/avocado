#ifndef AVOCADO_VULKAN_GRAPHICS_PIPELINE
#define AVOCADO_VULKAN_GRAPHICS_PIPELINE

#include "colorattachment.hpp"

#include "../errorstorage.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <vector>

namespace avocado::vulkan {

class GraphicsPipelineBuilder: public avocado::core::ErrorStorage {
public:
    enum class DynamicState {
        Viewport = VK_DYNAMIC_STATE_VIEWPORT,
        Scissor = VK_DYNAMIC_STATE_SCISSOR,
        LineWidth = VK_DYNAMIC_STATE_LINE_WIDTH,
        DepthBias = VK_DYNAMIC_STATE_DEPTH_BIAS,
        BlendConstants = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        DepthBounds = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        StencilCompareMask = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
        StencilWriteMask = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
        StencilReference = VK_DYNAMIC_STATE_STENCIL_REFERENCE,
        CullMode = VK_DYNAMIC_STATE_CULL_MODE,
        FrontFace = VK_DYNAMIC_STATE_FRONT_FACE,
        PrimitiveTopology = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
        ViewportWithCount = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        ScissorWithCount = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
        VertexInputBindingStride = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
        DepthTestEnable = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
        DepthWriteEnable = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
        DepthCompareOp = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
        DepthBoundsTestEnable = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE,
        StencilTestEnable = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
        StencilOp = VK_DYNAMIC_STATE_STENCIL_OP,
        RasterizerDiscardEnable = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE,
        DepthBiasEnable = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
        PrimitiveRestartEnable = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,
        ViewportWScalingNV = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,
        DiscardRectangleExt = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,
        SampleLocationsExt = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT,
        RayTracingPipelineStackSizeKHR = VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR,
        ViewportShadingRatePaletteNV = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV,
        ViewportCoarseSampleOrderNV = VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV,
        ExclusiveScissorNV = VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV,
        FragmentShadingRateKHR = VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR,
        LineStippleExt = VK_DYNAMIC_STATE_LINE_STIPPLE_EXT,
        VertexInputExt = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,
        PatchControlPointsExt = VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT,
        LogicOpExt = VK_DYNAMIC_STATE_LOGIC_OP_EXT,
        ColorWriteEnableExt = VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT,
        CullModeExt = VK_DYNAMIC_STATE_CULL_MODE_EXT,
        FrontFaceExt = VK_DYNAMIC_STATE_FRONT_FACE_EXT,
        PrimitiveTopologyExt = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT,
        ViewportWithCountExt = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT,
        ScissorWithCountExt = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT,
        VertexInputBindingStrideExt = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT,
        DepthTestEnableExt = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,
        DepthWriteEnableExt = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
        DepthCompareOpExt = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,
        DepthBoundsTestEnableExt = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT,
        StencilTestEnableExt = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT,
        StencilOpExt = VK_DYNAMIC_STATE_STENCIL_OP_EXT,
        RasterizerDiscardEnableExt = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT,
        DepthBiasEnableExt = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT,
        PrimitiveRestartEnableExt = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT,
   };

    explicit GraphicsPipelineBuilder(VkDevice device);

    // Dynamic state.
    void addDynamicStates(const std::initializer_list<DynamicState> &dynStates);
    void addDynamicState(DynamicState dynState);

    // Input assembly.
    enum class PrimitiveTopology {
        PointList = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        LineList = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        LineStrip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
        TriangleList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        TriangleStrip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        TriangleFan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
        LineListWithAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
        LineStripWithAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
        TriangleListWithAdjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
        TriangleStripWithAdjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
        PatchList = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
    };
    void setPrimitiveTopology(const PrimitiveTopology primTop);

    // Viewport.
    VkViewport getViewport() const;
    VkRect2D getScissor() const;
    void setViewPortSize(const uint32_t w, const uint32_t h);
    
    void setViewPortMaxDepth(const float maxDepth);

    // Rasterizer.
    enum class PolygonMode {
        Fill = VK_POLYGON_MODE_FILL,
        Line = VK_POLYGON_MODE_LINE,
        Point = VK_POLYGON_MODE_POINT,
        FillRectangleNV = VK_POLYGON_MODE_FILL_RECTANGLE_NV
    };

    void setPolygonMode(const PolygonMode polyMode);

    enum class CullMode: uint32_t {
        None = VK_CULL_MODE_NONE,
        FrontBit = VK_CULL_MODE_FRONT_BIT,
        BackBit = VK_CULL_MODE_BACK_BIT,
        FrontAndBack = VK_CULL_MODE_FRONT_AND_BACK,
    };
    void setCullMode(const CullMode cullMode);

    enum class FrontFace {
        CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        Clockwise = VK_FRONT_FACE_CLOCKWISE
    };
    void setFrontFace(const FrontFace ff);

    void setLineWidth(const float lineW);
    void setDepthClampEnable(const bool enable);
    void setRasterizerDiscardEnable(const bool enable);
    void setDepthBiasEnable(const bool enable);

    // Multisampling.
    enum class SampleCount {
        _1Bit = VK_SAMPLE_COUNT_1_BIT,
        _2Bit = VK_SAMPLE_COUNT_2_BIT,
        _4Bit = VK_SAMPLE_COUNT_4_BIT,
        _8Bit = VK_SAMPLE_COUNT_8_BIT,
        _16Bit = VK_SAMPLE_COUNT_16_BIT,
        _32Bit = VK_SAMPLE_COUNT_32_BIT,
        _64Bit = VK_SAMPLE_COUNT_64_BIT
    };
    void setSampleCount(const SampleCount sampleCnt);
    void setMinSampleShading(const float minSampleShading);

    // Color blending.
    enum class LogicOperation {
        Clear = VK_LOGIC_OP_CLEAR,
        And = VK_LOGIC_OP_AND,
        AndReverse = VK_LOGIC_OP_AND_REVERSE,
        Copy = VK_LOGIC_OP_COPY,
        AndInverted = VK_LOGIC_OP_AND_INVERTED,
        NoOp = VK_LOGIC_OP_NO_OP,
        Xor = VK_LOGIC_OP_XOR,
        Or = VK_LOGIC_OP_OR,
        Nor = VK_LOGIC_OP_NOR,
        Equivalent = VK_LOGIC_OP_EQUIVALENT,
        Invert = VK_LOGIC_OP_INVERT,
        OrReverse = VK_LOGIC_OP_OR_REVERSE,
        CopyInverted = VK_LOGIC_OP_COPY_INVERTED,
        OrInverted = VK_LOGIC_OP_OR_INVERTED,
        Nand = VK_LOGIC_OP_NAND,
        Set = VK_LOGIC_OP_SET
    };
    void setLogicOperation(const LogicOperation lo);
    void setLogicOperationEnable(const bool enable);

    void addColorAttachment(const ColorAttachment &ca);
    void addColorAttachment(ColorAttachment &&ca);

private:
    std::vector<VkDynamicState> _dynamicStates;
    uint32_t _dynamicStatesFlags = 0;

    // Input assembly. 
    PrimitiveTopology _primitiveTopology = PrimitiveTopology::PointList;

    // Viewport.
    uint32_t _viewPortWidth = 0, _viewPortHeight = 0;
    float _viewPortMaxDepth = 0.f;

    // Rasterizer.
    float _lineWidth = 1.f;
    bool _depthClamEnable = false, _rasterizerDiscardEnable = false, _depthBiasEnable = false;
    PolygonMode _polygonMode = PolygonMode::Point;
    FrontFace _frontFace = FrontFace::Clockwise;
    CullMode _cullMode = CullMode::None;

    // Multisampling.
    float _minSampleShading = 1.f;
    SampleCount _sampleCount = SampleCount::_1Bit;

    // Color blending.
    std::vector<ColorAttachment> _colorAttachments;
    LogicOperation _logicOperation = LogicOperation::NoOp;
    bool _logicOperationEnable = false;

    VkDevice _device;

public:
    using PipelineDestroyer = decltype(std::bind(vkDestroyPipeline, _device, std::placeholders::_1, nullptr));
    using PipelineUniquePtr = std::unique_ptr<std::remove_pointer_t<VkPipeline>, PipelineDestroyer>;
    PipelineUniquePtr buildPipeline(const std::vector<VkPipelineShaderStageCreateInfo> &shaderStageCIs, VkFormat format, VkRenderPass renderPass);

};

} // namespace avocado::vulkan.

#endif

