#ifndef AVOCADO_UTILS
#define AVOCADO_UTILS

#include "logger.hpp"

#include <vulkan/vkutils.hpp>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <type_traits>
#include <vector>

#define MAKE_COPYABLE(classname)\
    classname(const classname&) = default;\
    classname& operator=(const classname&) = default

#define MAKE_MOVABLE(classname)\
    classname(classname&&) = default;\
    classname& operator=(classname&&) = default

#define NON_COPYABLE(classname)\
    classname(const classname&) = delete;\
    classname& operator=(const classname&) = delete

#define NON_MOVABLE(classname)\
    classname(classname&&) = delete;\
    classname& operator=(classname&&) = delete

namespace avocado::utils {

template<typename T, typename ...Ts>
constexpr bool areOfSameType() {
    return std::conjunction_v<std::is_same<T, Ts>...>;
}

template<typename ...Enums>
constexpr std::common_type_t<Enums...> enumBitwiseOr(Enums ...enums) noexcept {
    static_assert(std::conjunction_v<std::is_enum<Enums>...>, "All types must be of enum type.");
    static_assert(areOfSameType<Enums...>(), "All types must be the same.");
    return static_cast<std::common_type_t<Enums...>>((static_cast<std::underlying_type_t<Enums>>(enums) | ...));
}

std::vector<char> readFile(const std::string &filePath);

[[nodiscard]] bool endsWith(const std::string_view &string, const std::string_view &target);
inline bool hasExtension(const std::string_view &filename, const std::string_view &extension) {
    return endsWith(filename, extension);
}

[[nodiscard]] inline bool stringContains(const std::string &str, const std::string &value) {
    return (str.find(value) != std::string::npos);
}

std::vector<std::string> splitString(const std::string &str, const std::string &delimiter, const bool includeEmpty);

inline bool areFloatEq(const float a, const float b) noexcept {
    return fabs(a - b) < std::numeric_limits<float>::epsilon();
}

template <typename Container>
void makeUniqueContainer(Container &cont) {
    std::sort(cont.begin(), cont.end());
    const auto newEnd = std::unique(cont.begin(), cont.end());
    cont.erase(newEnd, cont.end());
}

template <typename Enum>
consteval std::underlying_type_t<Enum> enumToInteger(Enum enumValue) {
    static_assert(std::is_enum_v<Enum>, "Enum must be of enum type.");

    return static_cast<std::underlying_type_t<Enum>>(enumValue);
}

} // namespace avocado::utils.

/*
 * @brief Calls Vulkan API function and logs error if it hasn't returned VK_SUCCESS.
 *
 * @param variableName Name of the result variable you can use later.
 * @param Fn Vulkan function to call.
 * @param args Arguments for Vulkan function.
 */
#define CALL_VULKAN_AND_DEFINE_VARIABLE(variableName, Fn, ...)\
    const VkResult variableName = Fn(__VA_ARGS__);\
    if (VK_SUCCESS != variableName)\
        LOG_ERROR(std::string(#Fn) + " returned " + avocado::vulkan::getVkResultString(variableName))

/*
 * @brief Calls Vulkan API function, in case of fail it logs an error and calls 'return'.
 *
 * @param variableName Name of the result variable you can use later.
 * @param Fn Vulkan function to call.
 * @param args Arguments for Vulkan function.
 */
#define CALL_VULKAN_DEFINE_VARIABLE_AND_RETURN(variableName, Fn, ...)\
    const VkResult variableName = Fn(__VA_ARGS__);\
    if (VK_SUCCESS != variableName) {\
        LOG_ERROR(std::string(#Fn) + " returned " + avocado::vulkan::getVkResultString(variableName));\
        return;\
    }\

/*
 * @brief Calls Vulkan API function, in case of fail it logs an error and calls 'return' with specified value.
 *
 * @param variableName Name of the result variable you can use later.
 * @param returnValue Value to return in case of fail.
 * @param Fn Vulkan function to call.
 * @param args Arguments for Vulkan function.
 */
#define CALL_VULKAN_DEFINE_VARIABLE_AND_RETURN_VALUE(variableName, returnValue, Fn, ...)\
    const VkResult variableName = Fn(__VA_ARGS__);\
    if (VK_SUCCESS != variableName) {\
        LOG_ERROR(std::string(#Fn) + " returned " + avocado::vulkan::getVkResultString(variableName));\
        return returnValue;\
    }\

/*
 * @brief Calls Vulkan API function and logs error if it hasn't returned VK_SUCCESS.
 *
 * @param Fn Vulkan function to call.
 * @param args Arguments for Vulkan function.
 *
 * @note This macro defines variable 'VkResult callResult'.
 */
#define CALL_VULKAN(Fn, ...) CALL_VULKAN_AND_DEFINE_VARIABLE(callResult, Fn, __VA_ARGS__)

/*
 * @brief Calls Vulkan API function, in case of fail it logs an error and calls 'return'.
 *
 * @param Fn Vulkan function to call.
 * @param args Arguments for Vulkan function.
 *
 * @note This macro defines variable 'VkResult callResult'.
 */
#define CALL_VULKAN_AND_RETURN(Fn, ...) CALL_VULKAN_DEFINE_VARIABLE_AND_RETURN(callResult, Fn, __VA_ARGS__)

/*
 * @brief Calls Vulkan API function, in case of fail it logs an error and calls 'return' with specified value.
 *
 * @param returnValue Value to return in case of fail.
 * @param Fn Vulkan function to call.
 * @param args Arguments for Vulkan function.
 *
 * @note This macro defines variable 'VkResult callResult'.
 */
#define CALL_VULKAN_AND_RETURN_VALUE(returnValue, Fn, ...) CALL_VULKAN_DEFINE_VARIABLE_AND_RETURN_VALUE(callResult, returnValue, Fn, __VA_ARGS__)

#endif
