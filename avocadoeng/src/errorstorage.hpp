#ifndef AVOCADO_CORE_ERRORSTORAGE
#define AVOCADO_CORE_ERRORSTORAGE

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <string>

namespace avocado {

namespace core {

class ErrorStorage {
public:
    bool hasError() const noexcept;
    const std::string &getErrorMessage() const noexcept;

protected:
    void setHasError(bool he) const noexcept;
    void setErrorMessage(const std::string &msg) const;
    void setErrorMessage(std::string &&msg) const noexcept;

    constexpr const char* getVkResultString(const VkResult vkres) const noexcept {
    #define PROCESS_CODE(code) case VK_ ##code: { return #code; }
    switch (vkres) {
        PROCESS_CODE(SUCCESS)
        PROCESS_CODE(NOT_READY)
        PROCESS_CODE(TIMEOUT)
        PROCESS_CODE(EVENT_SET)
        PROCESS_CODE(EVENT_RESET)
        PROCESS_CODE(INCOMPLETE)
        PROCESS_CODE(ERROR_OUT_OF_HOST_MEMORY)
        PROCESS_CODE(ERROR_OUT_OF_DEVICE_MEMORY)
        PROCESS_CODE(ERROR_INITIALIZATION_FAILED)
        PROCESS_CODE(ERROR_DEVICE_LOST)
        PROCESS_CODE(ERROR_MEMORY_MAP_FAILED)
        PROCESS_CODE(ERROR_LAYER_NOT_PRESENT)
        PROCESS_CODE(ERROR_EXTENSION_NOT_PRESENT)
        PROCESS_CODE(ERROR_FEATURE_NOT_PRESENT)
        PROCESS_CODE(ERROR_INCOMPATIBLE_DRIVER)
        PROCESS_CODE(ERROR_TOO_MANY_OBJECTS)
        PROCESS_CODE(ERROR_FORMAT_NOT_SUPPORTED)
        PROCESS_CODE(ERROR_FRAGMENTED_POOL)
        PROCESS_CODE(ERROR_UNKNOWN)
        PROCESS_CODE(ERROR_OUT_OF_POOL_MEMORY)
        PROCESS_CODE(ERROR_INVALID_EXTERNAL_HANDLE)
        PROCESS_CODE(ERROR_FRAGMENTATION)
        PROCESS_CODE(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
        PROCESS_CODE(PIPELINE_COMPILE_REQUIRED)
        PROCESS_CODE(ERROR_SURFACE_LOST_KHR)
        PROCESS_CODE(ERROR_NATIVE_WINDOW_IN_USE_KHR)
        PROCESS_CODE(SUBOPTIMAL_KHR)
        PROCESS_CODE(ERROR_OUT_OF_DATE_KHR)
        PROCESS_CODE(ERROR_INCOMPATIBLE_DISPLAY_KHR)
        PROCESS_CODE(ERROR_VALIDATION_FAILED_EXT)
        PROCESS_CODE(ERROR_INVALID_SHADER_NV)
        PROCESS_CODE(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
        PROCESS_CODE(ERROR_NOT_PERMITTED_KHR)
        PROCESS_CODE(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
        PROCESS_CODE(THREAD_IDLE_KHR)
        PROCESS_CODE(THREAD_DONE_KHR)
        PROCESS_CODE(OPERATION_DEFERRED_KHR)
        PROCESS_CODE(OPERATION_NOT_DEFERRED_KHR)
        PROCESS_CODE(RESULT_MAX_ENUM)
        default: { assert(false); return "UNKNOWN_ERROR"; }
    }

    // All code here is unreachable.
    return "UNREACHABLE_ERROR";
    #undef PROCESS_CODE
}
private:
    mutable std::string _msg;
    mutable bool _hasError = false;
};

} // namespace core.

} // namespace avocado.

#endif

