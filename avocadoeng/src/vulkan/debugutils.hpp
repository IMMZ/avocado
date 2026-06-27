#ifndef AVOCADO_VULKAN_DEBUGUTILS
#define AVOCADO_VULKAN_DEBUGUTILS

#include "logicaldevice.hpp"
#include "vkutils.hpp"
#include "structuretypes.hpp"

#include "../utils.hpp"

#include <memory>

namespace avocado::vulkan {

class DebugUtils {
public:
    template <typename T>
    void setObjectName(T object, const char *objectName) noexcept {
        static_assert(ObjectType<T> != VK_OBJECT_TYPE_MAX_ENUM, "This type is not supported");

        assert(_dev.getHandle() != VK_NULL_HANDLE);

        DEFINE_VK_STRUCTURE(VkDebugUtilsObjectNameInfoEXT, objNameInfo);
        objNameInfo.objectType = ObjectType<T>;
        objNameInfo.objectHandle = reinterpret_cast<uint64_t>(object);
        objNameInfo.pObjectName = objectName;
        auto fnPointer = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(_dev.getHandle(), "vkSetDebugUtilsObjectNameEXT"));
        assert(fnPointer != nullptr && "Can't get device process address for vkSetDebugUtilsObjectNameEXT");
        CALL_VULKAN(fnPointer, _dev.getHandle(), &objNameInfo);
    }

    template <typename T, typename Tag>
    void setObjectTag(T object, const uint64_t tagName, const Tag *tag, const size_t tagSize) noexcept {
        DEFINE_VK_STRUCTURE(VkDebugUtilsObjectTagInfoEXT, tagInfo);
        tagInfo.objectType = ObjectType<T>;
        tagInfo.objectHandle = reinterpret_cast<uint64_t>(object);
        tagInfo.tagName = tagName;
        tagInfo.tagSize = tagSize;
        tagInfo.pTag = tag;
        auto fnPointer = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(vkGetDeviceProcAddr(_dev.getHandle(), "vkSetDebugUtilsObjectTagEXT"));
        assert(fnPointer != nullptr && "Can't get device process address for vkSetDebugUtilsObjectTagEXT");
        CALL_VULKAN(fnPointer, _dev.getHandle(), &tagInfo);
    }

private:
    DebugUtils(LogicalDevice &device);

    LogicalDevice &_dev;

    friend std::unique_ptr<DebugUtils> LogicalDevice::createDebugUtils();
};

}

#endif

