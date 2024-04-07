#ifndef AVOCADO_VULKAN_CLIPPING
#define AVOCADO_VULKAN_CLIPPING

#include <vulkan/vulkan_core.h>

#include <cassert>

namespace avocado::vulkan {

struct Clipping {
    inline static VkRect2D createScissor(const int32_t x, const int32_t y, const uint32_t w, const uint32_t h) noexcept {
        return {{x, y}, {w, h}};
    }

    inline static VkRect2D createScissor(const VkViewport &viewport) noexcept {
        return createScissor(static_cast<int32_t>(viewport.x), static_cast<int32_t>(viewport.y),
            static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height));
    }

    inline static VkViewport createViewport(const float x, const float y, const float w, const float h, const float minDepth = 0.f, const float maxDepth = 1.f) noexcept {
        assert((minDepth < maxDepth) && "Min depth of viewport must be < than max depth.");
        return {x, y, w, h, minDepth, maxDepth};
    }

    static VkViewport createViewport(const float x, const float y, const VkExtent2D extent, const float minDepth = 0.f, const float maxDepth = 1.f) noexcept {
        assert((minDepth < maxDepth) && "Min depth of viewport must be < than max depth.");
        return createViewport(x, y, static_cast<float>(extent.width), static_cast<float>(extent.height), minDepth, maxDepth);
    }
};

}

#endif

