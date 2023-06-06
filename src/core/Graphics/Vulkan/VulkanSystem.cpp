// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanSystem.h"

#include "Graphics/Common.h"
#include "VulkanDevice.h"

namespace chronicle::graphics::internal::vulkan
{

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                             [[maybe_unused]] void *pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        CHRLOG_ERROR("{}", pCallbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        CHRLOG_WARN("{}", pCallbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        CHRLOG_INFO("{}", pCallbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        CHRLOG_DEBUG("{}", pCallbackData->pMessage);
    }
    return 0;
}

const std::vector<const char *> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const std::string EngineName = "Chronicle";

void VulkanSystem::init(const SystemInitInfo &systemInitInfo)
{
    assert(!_instance);

    _enableDebug = systemInitInfo.enableDebug;

    if (_enableDebug && !checkValidationLayerSupport(ValidationLayers))
    {
        _enableDebug = false;
        throw GraphicsError("Validation layers requested, but not available");
    }

    vk::ApplicationInfo appInfo(systemInitInfo.applicationName.c_str(), VK_MAKE_VERSION(1, 0, 0), EngineName.c_str(),
                                VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

    auto extensions = getRequiredExtensions();
    auto layers = enabledLayers();

    vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo, layers, extensions);

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (_enableDebug)
    {
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.setPNext(&debugCreateInfo);
    }

    // create the instance
    _instance = vk::createInstance(createInfo, nullptr);

    assert(_instance);
}

void VulkanSystem::deinit()
{
    assert(_instance);

    _instance.destroy();
    _instance = nullptr;
}

std::shared_ptr<Device> VulkanSystem::requestDevice(const RequestDeviceInfo &requestDeviceInfo)
{
    assert(_instance);

    auto devices = _instance.enumeratePhysicalDevices();
    if (devices.empty())
        throw GraphicsError("Failed to find GPUs with Vulkan support");

// this is a temporary surface, used only for check id a defice is suitable.
#ifdef CHR_PLATFORM_WIN32
    auto surface = _instance.createWin32SurfaceKHR(vk::Win32SurfaceCreateInfoKHR(
        vk::Win32SurfaceCreateFlagsKHR{}, nullptr, static_cast<HWND>(requestDeviceInfo.hwnd)));
#endif

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device, surface, DeviceExtensions))
        {
            auto families = findQueueFamilies(device, surface);
            _instance.destroySurfaceKHR(surface);
            auto vulkanDevice = std::make_shared<VulkanDevice>(device, families);
            vulkanDevice->_thisWeakPtr = vulkanDevice;
            return vulkanDevice;
        }
    }

    _instance.destroySurfaceKHR(surface);
    throw GraphicsError("Failed to find a suitable device");
}

std::vector<const char *> VulkanSystem::enabledExtensions()
{
    return DeviceExtensions;
}

std::vector<const char *> VulkanSystem::enabledLayers()
{
    std::vector<const char *> layers{};
    if (_enableDebug)
    {
        layers.insert(layers.end(), ValidationLayers.begin(), ValidationLayers.end());
    }
    return layers;
}

bool VulkanSystem::checkValidationLayerSupport(const std::vector<const char *> &validationLayers)
{
    // check validation layer support
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

std::vector<const char *> VulkanSystem::getRequiredExtensions()
{
    std::vector<const char *> extensions{};

    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef CHR_PLATFORM_WIN32
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    if (_enableDebug)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanSystem::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    createInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                              vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
    createInfo.setPfnUserCallback(debugCallback);
}

bool VulkanSystem::isDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface,
                                    const std::vector<const char *> &extensions)
{
    // find queue families
    VulkanQueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    // check if the extensions are supported
    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, extensions);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // get supported features
    const auto supportedFeatures = physicalDevice.getFeatures();

    // check and return result
    return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy &&
           supportedFeatures.fillModeNonSolid;
}

VulkanQueueFamilyIndices VulkanSystem::findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    // get family properties
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    // populate the queue family indices
    int i = 0;
    VulkanQueueFamilyIndices indices;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if (queueFamily.queueCount > 0 && physicalDevice.getSurfaceSupportKHR(i, surface))
            indices.presentFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

bool VulkanSystem::checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice,
                                               const std::vector<const char *> &extensions)
{
    // copy the required extensions into a set
    std::set<std::string, std::less<>> requiredExtensions(extensions.begin(), extensions.end());

    // enumerate all the device extensions and remove from the set
    for (const auto &extension : physicalDevice.enumerateDeviceExtensionProperties())
        requiredExtensions.erase(extension.extensionName);

    // if there's still an extension into the set, the device doesn't support all the extensions.
    return requiredExtensions.empty();
}

VulkanSwapChainSupportDetails VulkanSystem::querySwapChainSupport(vk::PhysicalDevice physicalDevice,
                                                                  vk::SurfaceKHR surface)
{
    VulkanSwapChainSupportDetails details;
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    return details;
}

} // namespace chronicle::graphics::internal::vulkan