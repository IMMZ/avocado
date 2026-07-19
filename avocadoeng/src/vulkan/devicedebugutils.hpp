// This file is just a separation for debug features of Vulkan device.
// It's intended to be included in LogicalDevice class' header.
template <typename T>
void setObjectName(T object, const char *objectName) noexcept {
    static_assert(ObjectType<T> != VK_OBJECT_TYPE_MAX_ENUM, "This type is not supported");

    DEFINE_VK_STRUCTURE(VkDebugUtilsObjectNameInfoEXT, objNameInfo);
    objNameInfo.objectType = ObjectType<T>;
    objNameInfo.objectHandle = reinterpret_cast<uint64_t>(object);
    objNameInfo.pObjectName = objectName;
    auto fnPointer = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(_dev.get(), "vkSetDebugUtilsObjectNameEXT"));
    if (fnPointer) {
        CALL_VULKAN(fnPointer, _dev.get(), &objNameInfo);
    }
}

template <typename T, typename Tag>
void setObjectTag(T object, const uint64_t tagName, const Tag *tag, const size_t tagSize) noexcept {
    static_assert(ObjectType<T> != VK_OBJECT_TYPE_MAX_ENUM, "This type is not supported");

    DEFINE_VK_STRUCTURE(VkDebugUtilsObjectTagInfoEXT, tagInfo);
    tagInfo.objectType = ObjectType<T>;
    tagInfo.objectHandle = reinterpret_cast<uint64_t>(object);
    tagInfo.tagName = tagName;
    tagInfo.tagSize = tagSize;
    tagInfo.pTag = tag;
    auto fnPointer = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(vkGetDeviceProcAddr(_dev.get(), "vkSetDebugUtilsObjectTagEXT"));
    if (fnPointer) {
        CALL_VULKAN(fnPointer, _dev.get(), &tagInfo);
    }
}
