#include "colorattachment.hpp"

namespace avocado::vulkan {

ColorAttachment::ColorComponent ColorAttachment::getColorComponent() const {
    return _colorComponent;
}

void ColorAttachment::setColorComponent(const ColorComponent cc) {
    _colorComponent = cc;
}

bool ColorAttachment::isBlendEnabled() const {
    return _blendEnable;
}

void ColorAttachment::setBlendEnable(const bool enable) {
    _blendEnable = enable;
}

ColorAttachment::BlendFactor ColorAttachment::getSrcBlendFactor() const {
    return _srcBlendFactor;
}

void ColorAttachment::setSrcBlendFactor(const BlendFactor bf) {
    _srcBlendFactor = bf;
}

ColorAttachment::BlendFactor ColorAttachment::getDstBlendFactor() const {
    return _dstBlendFactor;
}

void ColorAttachment::setDstBlendFactor(const BlendFactor bf) {
    _dstBlendFactor = bf;
}

ColorAttachment::BlendFactor ColorAttachment::getSrcAlphaBlendFactor() const {
    return _srcAlphaBlendFactor;
}

void ColorAttachment::setSrcAlphaBlendFactor(const BlendFactor bf) {
    _srcAlphaBlendFactor = bf;
}

ColorAttachment::BlendFactor ColorAttachment::getDstAlphaBlendFactor() const {
    return _dstAlphaBlendFactor;
}

void ColorAttachment::setDstAlphaBlendFactor(const BlendFactor bf) {
    _dstAlphaBlendFactor = bf;
}

ColorAttachment::BlendOperation ColorAttachment::getColorBlendOperation() const {
    return _colorBlendOp;
}

void ColorAttachment::setColorBlendOperation(const BlendOperation bo) {
    _colorBlendOp = bo;
}

ColorAttachment::BlendOperation ColorAttachment::getAlphaBlendOperation() const {
    return _alphaBlendOp;
}

void ColorAttachment::setAlphaBlendOperation(const BlendOperation bo) {
    _alphaBlendOp = bo;
}

} // namespace avocado::vulkan.

