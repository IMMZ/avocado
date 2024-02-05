#ifndef AVOCADO_VULKAN_DEBUGUTILS
#define AVOCADO_VULKAN_DEBUGUTILS

#include "logicaldevice.hpp"
#include "vkutils.hpp"

#include "internal/structuretypes.hpp"

#include <memory>

namespace avocado::vulkan {

class DebugUtils: public core::ErrorStorage {
public:
    template <typename T>
    void setObjectName(T object, const char *objectName) noexcept {
        static_assert(internal::ObjectType<T> != VK_OBJECT_TYPE_MAX_ENUM, "This type is not supported");

        assert(_dev.getHandle() != VK_NULL_HANDLE);

        auto objNameInfo = createStruct<VkDebugUtilsObjectNameInfoEXT>();
        objNameInfo.objectType = internal::ObjectType<T>;
        objNameInfo.objectHandle = reinterpret_cast<uint64_t>(object);
        objNameInfo.pObjectName = objectName;
        auto fnPointer = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(_dev.getHandle(), "vkSetDebugUtilsObjectNameEXT"));
        if (fnPointer != nullptr) {
            const VkResult result = fnPointer(_dev.getHandle(), &objNameInfo);
            setHasError(result != VK_SUCCESS);
            if (hasError())
                setErrorMessage("vkSetDebugUtilsObjectNameEXT returned "s + getVkResultString(result));
        } else {
            setHasError(true);
            setErrorMessage("Can't get device process address for vkSetDebugUtilsObjectNameEXT");
        }
    }

    template <typename T, typename Tag>
    void setObjectTag(T object, const uint64_t tagName, const Tag *tag, const size_t tagSize) noexcept {
        auto tagInfo = createStruct<VkDebugUtilsObjectTagInfoEXT>();
        tagInfo.objectType = internal::ObjectType<T>;
        tagInfo.objectHandle = reinterpret_cast<uint64_t>(object);
        tagInfo.tagName = tagName;
        tagInfo.tagSize = tagSize;
        tagInfo.pTag = tag;
        auto fnPointer = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(vkGetDeviceProcAddr(_dev.getHandle(), "vkSetDebugUtilsObjectTagEXT"));
        if (fnPointer != nullptr) {
            const VkResult result = fnPointer(_dev.getHandle(), &tagInfo);
            setHasError(result != VK_SUCCESS);
            if (hasError())
                setErrorMessage("vkSetDebugUtilsObjectTagEXT returned "s + getVkResultString(result));
        } else {
            setHasError(true);
            setErrorMessage("Can't get device process address for vkSetDebugUtilsObjectTagEXT");
        }
    }

private:
    DebugUtils(LogicalDevice &device);

    LogicalDevice &_dev;

    friend std::unique_ptr<DebugUtils> LogicalDevice::createDebugUtils();
};

}

#endif

