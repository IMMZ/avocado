#ifndef AVOCADO_VULKAN_OBJECTDELETER
#define AVOCADO_VULKAN_OBJECTDELETER

#include <vulkan/vulkan.h>

#include <memory>

namespace avocado::vulkan {

class LogicalDevice;

namespace internal {

template <typename T>
void destroyObject(LogicalDevice &logicalDevice, T objHandle);

} // namespace internal.

template <typename T>
struct ObjectDeleter {
    ObjectDeleter(LogicalDevice &logicalDevice):
        _logicalDevice(logicalDevice){}

    void operator()(T objectHandle) {
        internal::destroyObject(_logicalDevice, objectHandle);
    }

private:
    LogicalDevice &_logicalDevice;
};

template <typename T>
using ObjectPtr = std::unique_ptr<std::remove_pointer_t<T>, ObjectDeleter<T>>;

}
#endif // AVOCADO_VULKAN_OBJECTDELETER
