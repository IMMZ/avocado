#ifndef AVOCADO_CORE_VECN
#define AVOCADO_CORE_VECN

#include <vulkan/vulkan_core.h>

#include <cstdint>

namespace avocado::glsl {

namespace internal {

template <typename T>
struct vec2 {
    T x = T(), y = T();
};

template <typename T>
struct vec3 {
    T x = T(), y = T(), z = T();
};

template <typename T>
struct vec4 {
    T x = T(), y = T(), z = T(), w = T();
};

} // namespace internal.

using Int = int32_t;
using UInt = uint32_t;
using ivec2 = internal::vec2<int32_t>;
using ivec3 = internal::vec3<int32_t>;
using ivec4 = internal::vec4<int32_t>;
using uvec2 = internal::vec2<uint32_t>;
using uvec3 = internal::vec3<uint32_t>;
using uvec4 = internal::vec4<uint32_t>;
using vec2 = internal::vec2<float>;
using vec3 = internal::vec3<float>;
using vec4 = internal::vec4<float>;
using dvec2 = internal::vec2<double>;
using dvec3 = internal::vec3<double>;
using dvec4 = internal::vec4<double>;

template <typename T>
inline constexpr VkFormat FormatForGLSLType = VK_FORMAT_MAX_ENUM;

template <> inline constexpr VkFormat FormatForGLSLType<Int> = VK_FORMAT_R32_SINT;
template <> inline constexpr VkFormat FormatForGLSLType<UInt> = VK_FORMAT_R32_UINT;
template <> inline constexpr VkFormat FormatForGLSLType<ivec2> = VK_FORMAT_R32G32_SINT;
template <> inline constexpr VkFormat FormatForGLSLType<ivec3> = VK_FORMAT_R32G32B32_SINT;
template <> inline constexpr VkFormat FormatForGLSLType<ivec4> = VK_FORMAT_R32G32B32A32_SINT;
template <> inline constexpr VkFormat FormatForGLSLType<uvec2> = VK_FORMAT_R32G32_UINT;
template <> inline constexpr VkFormat FormatForGLSLType<uvec3> = VK_FORMAT_R32G32B32_UINT;
template <> inline constexpr VkFormat FormatForGLSLType<uvec4> = VK_FORMAT_R32G32B32A32_UINT;
template <> inline constexpr VkFormat FormatForGLSLType<vec2> = VK_FORMAT_R32G32_SFLOAT;
template <> inline constexpr VkFormat FormatForGLSLType<vec3> = VK_FORMAT_R32G32B32_SFLOAT;
template <> inline constexpr VkFormat FormatForGLSLType<vec4> = VK_FORMAT_R32G32B32A32_SFLOAT;
template <> inline constexpr VkFormat FormatForGLSLType<dvec2> = VK_FORMAT_R64G64_SFLOAT;
template <> inline constexpr VkFormat FormatForGLSLType<dvec3> = VK_FORMAT_R64G64_SFLOAT;
template <> inline constexpr VkFormat FormatForGLSLType<dvec4> = VK_FORMAT_R64G64B64A64_SFLOAT;

} // namespace avocado.

#endif

