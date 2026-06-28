#ifndef CALLVULKAN_HPP
#define CALLVULKAN_HPP

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
        LOG_ERROR(std::string(#Fn) + " returned " + avocado::vulkan::utils::getVkResultString(variableName))

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
        LOG_ERROR(std::string(#Fn) + " returned " + avocado::vulkan::utils::getVkResultString(variableName));\
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
        LOG_ERROR(std::string(#Fn) + " returned " + avocado::vulkan::utils::getVkResultString(variableName));\
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

#endif // ifndef CALLVULKAN_HPP
