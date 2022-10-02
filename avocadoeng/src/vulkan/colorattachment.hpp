#ifndef AVOCADO_VULKAN_COLORATTACHMENT
#define AVOCADO_VULKAN_COLORATTACHMENT

#include <vulkan/vulkan.h>

namespace avocado::vulkan {

class ColorAttachment {
public:
    enum class ColorComponent {
        R = VK_COLOR_COMPONENT_R_BIT,
        G = VK_COLOR_COMPONENT_G_BIT,
        B = VK_COLOR_COMPONENT_B_BIT,
        A = VK_COLOR_COMPONENT_A_BIT
    };
    ColorComponent getColorComponent() const;
    void setColorComponent(const ColorComponent cc);
    bool isBlendEnabled() const;
    void setBlendEnable(const bool enable);

    enum class BlendFactor {
        Zero = VK_BLEND_FACTOR_ZERO,
        One = VK_BLEND_FACTOR_ONE,
        SrcColor = VK_BLEND_FACTOR_SRC_COLOR,
        OneMinusSrcColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        DstColor = VK_BLEND_FACTOR_DST_COLOR,
        OneMinusDstColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        SrcAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
        OneMinusSrc_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        DstAlpha = VK_BLEND_FACTOR_DST_ALPHA,
        OneMinusDstAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
        ConstantColor = VK_BLEND_FACTOR_CONSTANT_COLOR,
        OneMinusConstantColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
        ConstantAlpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
        OneMinusConstantAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
        SrcAlphaSaturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
        Src1Color = VK_BLEND_FACTOR_SRC1_COLOR,
        OneMinusSrc1Color = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
        Src1Alpha = VK_BLEND_FACTOR_SRC1_ALPHA,
        OneMinusSrc1Alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
    };
    BlendFactor getSrcBlendFactor() const;
    void setSrcBlendFactor(const BlendFactor bf);
    BlendFactor getDstBlendFactor() const;
    void setDstBlendFactor(const BlendFactor bf);
    BlendFactor getSrcAlphaBlendFactor() const;
    void setSrcAlphaBlendFactor(const BlendFactor bf);
    BlendFactor getDstAlphaBlendFactor() const;
    void setDstAlphaBlendFactor(const BlendFactor bf);

    enum class BlendOperation {
        Add = VK_BLEND_OP_ADD,
        Subtract = VK_BLEND_OP_SUBTRACT,
        Reverse_Subtract = VK_BLEND_OP_REVERSE_SUBTRACT,
        Min = VK_BLEND_OP_MIN,
        Max = VK_BLEND_OP_MAX,
        ZeroExt = VK_BLEND_OP_ZERO_EXT,
        SrcExt = VK_BLEND_OP_SRC_EXT,
        DstExt = VK_BLEND_OP_DST_EXT,
        SrcOverExt = VK_BLEND_OP_SRC_OVER_EXT,
        DstOverExt = VK_BLEND_OP_DST_OVER_EXT,
        SrcInExt = VK_BLEND_OP_SRC_IN_EXT,
        DstInExt = VK_BLEND_OP_DST_IN_EXT,
        SrcOutExt = VK_BLEND_OP_SRC_OUT_EXT,
        DstOutExt = VK_BLEND_OP_DST_OUT_EXT,
        SrcAtopExt = VK_BLEND_OP_SRC_ATOP_EXT,
        DstAtopExt = VK_BLEND_OP_DST_ATOP_EXT,
        XorExt = VK_BLEND_OP_XOR_EXT,
        MultiplyExt = VK_BLEND_OP_MULTIPLY_EXT,
        ScreenExt = VK_BLEND_OP_SCREEN_EXT,
        OverlayExt = VK_BLEND_OP_OVERLAY_EXT,
        DarkenExt = VK_BLEND_OP_DARKEN_EXT,
        LightenExt = VK_BLEND_OP_LIGHTEN_EXT,
        ColorDodgeExt = VK_BLEND_OP_COLORDODGE_EXT,
        ColorBurnExt = VK_BLEND_OP_COLORBURN_EXT,
        HardlightExt = VK_BLEND_OP_HARDLIGHT_EXT,
        SoftlightExt = VK_BLEND_OP_SOFTLIGHT_EXT,
        DifferenceExt = VK_BLEND_OP_DIFFERENCE_EXT,
        ExclusionExt = VK_BLEND_OP_EXCLUSION_EXT,
        InvertExt = VK_BLEND_OP_INVERT_EXT,
        InvertRGBExt = VK_BLEND_OP_INVERT_RGB_EXT,
        LineardodgeExt = VK_BLEND_OP_LINEARDODGE_EXT,
        LinearburnExt = VK_BLEND_OP_LINEARBURN_EXT,
        VividlightExt = VK_BLEND_OP_VIVIDLIGHT_EXT,
        LinearlightExt = VK_BLEND_OP_LINEARLIGHT_EXT,
        PinlightExt = VK_BLEND_OP_PINLIGHT_EXT,
        HardmixExt = VK_BLEND_OP_HARDMIX_EXT,
        HSLHueExt = VK_BLEND_OP_HSL_HUE_EXT,
        HSLSaturationExt = VK_BLEND_OP_HSL_SATURATION_EXT,
        HSLColorExt = VK_BLEND_OP_HSL_COLOR_EXT,
        HSLLuminosityExt = VK_BLEND_OP_HSL_LUMINOSITY_EXT,
        PlusExt = VK_BLEND_OP_PLUS_EXT,
        PlusClampedExt = VK_BLEND_OP_PLUS_CLAMPED_EXT,
        PlusClamped_AlphaExt = VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT,
        PlusDarkerExt = VK_BLEND_OP_PLUS_DARKER_EXT,
        MINUSExt = VK_BLEND_OP_MINUS_EXT,
        MINUSClampedExt = VK_BLEND_OP_MINUS_CLAMPED_EXT,
        ContrastExt = VK_BLEND_OP_CONTRAST_EXT,
        InvertOVGExt = VK_BLEND_OP_INVERT_OVG_EXT,
        RedExt = VK_BLEND_OP_RED_EXT,
        Greenext = VK_BLEND_OP_GREEN_EXT,
        BlueExt = VK_BLEND_OP_BLUE_EXT,
    };
    BlendOperation getColorBlendOperation() const;
    void setColorBlendOperation(const BlendOperation bo);
    BlendOperation getAlphaBlendOperation() const;
    void setAlphaBlendOperation(const BlendOperation bo);

private:
    BlendFactor _srcBlendFactor = BlendFactor::Zero, _dstBlendFactor = _srcBlendFactor, _srcAlphaBlendFactor = _srcBlendFactor, _dstAlphaBlendFactor = _dstBlendFactor;
    BlendOperation _colorBlendOp = BlendOperation::Add, _alphaBlendOp = BlendOperation::Add;
    ColorComponent _colorComponent = ColorComponent::R;
    bool _blendEnable = false;
};

} // namespace avocado::vulkan.

#endif

