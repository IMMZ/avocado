#ifndef AVOCADO_VULKAN_OBJECTDELETER
#define AVOCADO_VULKAN_OBJECTDELETER

#include <vulkan/vulkan.h>

#include <memory>

namespace avocado::vulkan {

class LogicalDevice;

namespace internal {

template <typename T>
void destroyObject(LogicalDevice &logicalDevice, T objHandle);

template <typename T>
void destroyFundamentalObject(T objectHandle);

template <typename T>
void freeAllocation(LogicalDevice &device, T allocatedObjectHandle) noexcept;

} // namespace internal.

template <typename T>
struct ObjectDeleter {
    explicit ObjectDeleter(LogicalDevice &logicalDevice):
        _logicalDevice(logicalDevice){}

    void operator()(T objectHandle) {
        if (objectHandle != VK_NULL_HANDLE)
            internal::destroyObject(_logicalDevice, objectHandle);
    }

private:
    std::reference_wrapper<LogicalDevice> _logicalDevice;
};

template <typename T>
struct FundamentalObjectDeleter // todo rename to more sensible name. Both with internal function.
{
    void operator()(T objectHandle) {
        if (objectHandle != VK_NULL_HANDLE)
            internal::destroyFundamentalObject(objectHandle);
    }
};

template <typename T>
struct AllocatedObjectDeleter
{
    explicit AllocatedObjectDeleter(LogicalDevice &device):
        _logicalDevice(device) {
    }

    void operator()(T objectHandle) {
        if (objectHandle != VK_NULL_HANDLE)
            internal::freeAllocation(_logicalDevice, objectHandle);
    }

private:
    std::reference_wrapper<LogicalDevice> _logicalDevice;
};

template <typename T>
using ObjectPtr = std::unique_ptr<std::remove_pointer_t<T>, ObjectDeleter<T>>;

template <typename T>
using FundamentalObjectPtr = std::unique_ptr<std::remove_pointer_t<T>, FundamentalObjectDeleter<T>>;

template <typename T>
using AllocatedObjectPtr = std::unique_ptr<std::remove_pointer_t<T>, AllocatedObjectDeleter<T>>;

template <typename T>
ObjectPtr<T> makeObjectPtr(LogicalDevice &logicalDevice, T objHandle) {
    return ObjectPtr<T>(objHandle, ObjectDeleter<T>(logicalDevice));
}

template <typename T>
FundamentalObjectPtr<T> makeFundamentalObjectPtr(T objHandle) {
    return FundamentalObjectPtr<T>(objHandle, FundamentalObjectDeleter<T>());
}

template <typename T>
AllocatedObjectPtr<T> makeAllocatedObjectPtr(LogicalDevice &logicalDevice, T objHandle) {
    return AllocatedObjectPtr<T>(objHandle, AllocatedObjectDeleter<T>(logicalDevice));
}

} // namespace avocado::vulkan.

#endif // AVOCADO_VULKAN_OBJECTDELETER

