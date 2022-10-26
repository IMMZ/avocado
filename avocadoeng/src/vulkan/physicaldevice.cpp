#include "physicaldevice.hpp"

#include <cstring>

using namespace std::literals::string_literals;

namespace avocado::vulkan {

PhysicalDevice::PhysicalDevice():
    ErrorStorage(),
    _device(VK_NULL_HANDLE) {
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device):
    ErrorStorage(),
    _device(device) {
}

VkPhysicalDevice PhysicalDevice::getHandle() {
    return _device;
}

bool PhysicalDevice::isValid() const {
    return (_device != VK_NULL_HANDLE);
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilies() const {
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyPropertiesCount, nullptr);
    if (queueFamilyPropertiesCount > 0) {
        std::vector<VkQueueFamilyProperties> result(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyPropertiesCount, result.data());
        return result;
    }

    return std::vector<VkQueueFamilyProperties>();
}

uint32_t PhysicalDevice::getGraphicsQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilies) const {
    for (size_t i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return static_cast<uint32_t>(i);
        }
    }

    return std::numeric_limits<uint32_t>::max();
}

LogicalDevice PhysicalDevice::createLogicalDevice(
    const std::vector<uint32_t> &uniqueQueueFamilyIndices,
    const std::vector<std::string> &extensions,
    const std::vector<std::string> &instanceLayers,
    const uint32_t queueCount, const float queuePriority) {
    // Creating queuecreateinfos for graphics family.
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilyIndices.size());
    for (size_t i = 0; i < uniqueQueueFamilyIndices.size(); ++i) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamilyIndices[i];
        queueCreateInfos[i].queueCount = queueCount;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    std::vector<const char*> extensionsCString(extensions.size(), nullptr);
    for (size_t i = 0; i < extensionsCString.size(); ++i) {
        extensionsCString[i] = extensions[i].c_str();
    }

    VkDeviceCreateInfo devCreateInfo{};
    devCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devCreateInfo.queueCreateInfoCount = static_cast<decltype(devCreateInfo.queueCreateInfoCount)>(queueCreateInfos.size());
    devCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    devCreateInfo.enabledExtensionCount = static_cast<decltype(devCreateInfo.enabledExtensionCount)>(extensionsCString.size());
    devCreateInfo.ppEnabledExtensionNames = extensionsCString.data();

    std::vector<const char*> layerNamesCString(instanceLayers.size(), nullptr);
    for (size_t i = 0; i < layerNamesCString.size(); ++i) {
        layerNamesCString[i] = instanceLayers[i].c_str();
    }


    if (!instanceLayers.empty()) {
        devCreateInfo.enabledLayerCount = static_cast<uint32_t>(layerNamesCString.size());
        devCreateInfo.ppEnabledLayerNames = layerNamesCString.data();
    }

    VkDevice logicDevHandle;
    const VkResult createDeviceResult = vkCreateDevice(_device, &devCreateInfo, nullptr, &logicDevHandle);
    setHasError(createDeviceResult != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("vkCreateDevice() returned "s + getVkResultString(createDeviceResult));
        return LogicalDevice();
    }

    return LogicalDevice(logicDevHandle);
}

std::vector<std::string> PhysicalDevice::getPhysicalDeviceExtensions() const {
    std::vector<std::string> extensions;
    uint32_t count = std::numeric_limits<uint32_t>::max();
    const VkResult enumerateDevExt1 = vkEnumerateDeviceExtensionProperties(_device, nullptr, &count, nullptr);
    setHasError(enumerateDevExt1 != VK_SUCCESS);
    if (hasError()) {
        setErrorMessage("VkEnumerateDeviceExtensionProperties returned "s + getVkResultString(enumerateDevExt1));
        return extensions;
    }

    if (count > 0) {
        extensions.resize(count);
        std::vector<VkExtensionProperties> extProps(count);
        const VkResult enumerateDevExt2 = vkEnumerateDeviceExtensionProperties(_device, nullptr, &count, extProps.data());
        setHasError(enumerateDevExt2 != VK_SUCCESS);
        if (hasError()) {
            setErrorMessage("VkEnumerateDeviceExtensionProperties returned "s + getVkResultString(enumerateDevExt2));
            return extensions;
        }

        for (size_t i = 0; i < extProps.size(); ++i)
            extensions[i] = extProps[i].extensionName;
    }

    return extensions;
}

bool PhysicalDevice::areExtensionsSupported(const std::vector<std::string> &extNames) const {
    std::vector<std::string> deviceExtensions = getPhysicalDeviceExtensions(); 
    if (hasError()) {
        setErrorMessage("getDeviceExtensions() returned Error ("s + getErrorMessage() + ')');
        return false;
    }

    bool extFound = false;
    for (const std::string &extName: extNames) {
        extFound = false;
        for (const std::string &devExt: deviceExtensions) {
            if (devExt == extName) {
                extFound = true;
                break;
            }
        }

        if (!extFound) {
            setHasError(true);
            setErrorMessage("Physical extension '"s + extName + "' isn't supported");
            return false;
        }
    }

    return true;
}


} // namespace avocado::vulkan

