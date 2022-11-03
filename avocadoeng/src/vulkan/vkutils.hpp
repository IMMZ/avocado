#ifndef AVOCADO_VULKAN
#define AVOCADO_VULKAN

#include <vulkan/vulkan_core.h>


#include "internal/structuretypes.hpp"

namespace avocado::vulkan {

template <typename T>
constexpr T createStruct() {
    static_assert(internal::StructureType<T> != VK_STRUCTURE_TYPE_MAX_ENUM, "No VkStructureType provided for type T.");

    T t{};
    t.sType = internal::StructureType<T>;
    return t;
}

}

#endif

