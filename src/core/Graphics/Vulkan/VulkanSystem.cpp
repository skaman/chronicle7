// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanSystem.h"

#include "Graphics/Common.h"

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
const std::string EngineName = "Chronicle";

void VulkanSystem::init(const SystemInitInfo &systemInitInfo)
{
    _enableDebug = systemInitInfo.enableDebug;

    // check validation layers support
    if (_enableDebug && !checkValidationLayerSupport(ValidationLayers))
    {
        _enableDebug = false;
        throw GraphicsError("Validation layers requested, but not available");
    }

    // application info
    vk::ApplicationInfo appInfo = {};
    appInfo.setPApplicationName(systemInitInfo.applicationName.c_str());
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setPEngineName(EngineName.c_str());
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setApiVersion(VK_API_VERSION_1_0);

    // prepare create instance info
    auto extensions = getRequiredExtensions();
    vk::InstanceCreateInfo createInfo = {};
    createInfo.setPApplicationInfo(&appInfo);
    createInfo.setPEnabledExtensionNames(extensions);

    // add debug related stuff if validation layers are enabled
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (_enableDebug)
    {
        populateDebugMessengerCreateInfo(debugCreateInfo);

        createInfo.setPEnabledLayerNames(ValidationLayers);
        createInfo.setPNext(&debugCreateInfo);
    }

    // create the instance
    _instance = vk::createInstance(createInfo, nullptr);
}

void VulkanSystem::deinit()
{
    _instance.destroy();
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
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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

} // namespace chronicle::graphics::internal::vulkan