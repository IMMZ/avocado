#include "physicaldevice.hpp"

#include "surface.hpp"
#include "vkutils.hpp"

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

VkPhysicalDevice PhysicalDevice::getHandle() noexcept {
    return _device;
}

bool PhysicalDevice::isValid() const noexcept {
    return (_device != VK_NULL_HANDLE);
}

void PhysicalDevice::initQueueFamilies(Surface &surface) {
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyPropertiesCount, nullptr);
    if (queueFamilyPropertiesCount > 0) {
        std::vector<VkQueueFamilyProperties> result(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyPropertiesCount, result.data());
        VkBool32 presentSupport = VK_FALSE;
        for (size_t i = 0; i < result.size(); ++i) {
            if (_graphicsQueueFamily == std::numeric_limits<QueueFamily>::max() && (result[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                _graphicsQueueFamily = static_cast<uint32_t>(i);
            }

            if (_transferQueueFamily == std::numeric_limits<QueueFamily>::max() && (result[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {
                _transferQueueFamily = static_cast<uint32_t>(i);
            }

            if (_presentQueueFamily == std::numeric_limits<QueueFamily>::max()) {
                const VkResult surfSupportResult = vkGetPhysicalDeviceSurfaceSupportKHR(_device, static_cast<uint32_t>(i), surface.getHandle(), &presentSupport);
                setHasError(surfSupportResult != VK_SUCCESS);
                if (hasError()) {
                    setErrorMessage("vkGetPhysicalDeviceSurfaceSupportKHR returned "s + getVkResultString(surfSupportResult));
                }

                if (presentSupport == VK_TRUE)
                    _presentQueueFamily = static_cast<uint32_t>(i);
            }
        }
    }
}

QueueFamily PhysicalDevice::getGraphicsQueueFamily() const noexcept {
    return _graphicsQueueFamily;
}

QueueFamily PhysicalDevice::getPresentQueueFamily() const noexcept {
    return _presentQueueFamily;
}

QueueFamily PhysicalDevice::getTransferQueueFamily() const noexcept {
    return _transferQueueFamily;
}

LogicalDevice PhysicalDevice::createLogicalDevice(
    const std::vector<uint32_t> &uniqueQueueFamilyIndices,
    const std::vector<std::string> &extensions,
    const std::vector<std::string> &instanceLayers,
    const uint32_t queueCount, const float queuePriority) {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(
        uniqueQueueFamilyIndices.size(), createStruct<VkDeviceQueueCreateInfo>());
    for (size_t i = 0; i < uniqueQueueFamilyIndices.size(); ++i) {
        queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamilyIndices[i];
        queueCreateInfos[i].queueCount = queueCount;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    std::vector<const char*> extensionsCString(extensions.size(), nullptr);
    for (size_t i = 0; i < extensionsCString.size(); ++i) {
        extensionsCString[i] = extensions[i].c_str();
    }

    auto devCreateInfo = createStruct<VkDeviceCreateInfo>();
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

    LogicalDevice logicalDevice(logicDevHandle);
    logicalDevice.setQueueFamilies(getGraphicsQueueFamily(), getPresentQueueFamily(), getTransferQueueFamily());
    return logicalDevice;
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

