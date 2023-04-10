#include "VulkanInstance.h"

#include "Platform/App.h"

#include "VulkanCommandBuffer.h"
#include "VulkanFence.h"
#include "VulkanImage.h"
#include "VulkanInstance.h"
#include "VulkanSemaphore.h"

const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool ENABLED_VALIDATION_LAYERS = false;
#else
const bool ENABLED_VALIDATION_LAYERS = true;
#endif

namespace chronicle {

CHR_CONCRETE(VulkanInstance)

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    return func != nullptr ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, callback, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CHRLOG_ERROR("{}", pCallbackData->pMessage)
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        CHRLOG_WARN("{}", pCallbackData->pMessage)
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        CHRLOG_INFO("{}", pCallbackData->pMessage)
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        CHRLOG_DEBUG("{}", pCallbackData->pMessage)
    }
    return 0;
}

VulkanInstance::VulkanInstance(chronicle::App* app)
    : _app(app)
{
    CHRZONE_VULKAN

    createInstance();
    setupDebugCallback();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createCommandPool();
}

VulkanInstance::~VulkanInstance()
{
    CHRZONE_VULKAN

    VulkanContext::depthImage.reset();
    VulkanContext::swapChainImages.clear();

    VulkanContext::device.destroySwapchainKHR(VulkanContext::swapChain);
    VulkanContext::device.destroyCommandPool(VulkanContext::commandPool);
    VulkanContext::device.destroy();

    if (ENABLED_VALIDATION_LAYERS) {
        destroyDebugUtilsMessengerEXT(VulkanContext::instance, VulkanContext::debugCallback, nullptr);
    }

    VulkanContext::instance.destroySurfaceKHR(VulkanContext::surface);
    VulkanContext::instance.destroy();
}

void VulkanInstance::waitIdle() const
{
    CHRZONE_VULKAN

    VulkanContext::device.waitIdle();
}

void VulkanInstance::waitForFence(const FenceRef& fence) const
{
    CHRZONE_VULKAN

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());
    (void)VulkanContext::device.waitForFences(vulkanFence->fence(), true, std::numeric_limits<uint64_t>::max());
}

void VulkanInstance::resetFence(const FenceRef& fence) const
{
    CHRZONE_VULKAN

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());
    (void)VulkanContext::device.resetFences(vulkanFence->fence());
}

uint32_t VulkanInstance::acquireNextImage(const SemaphoreRef& semaphore)
{
    CHRZONE_VULKAN

    const auto vulkanSemaphore = static_cast<const VulkanSemaphore*>(semaphore.get());

    try {
        auto result = VulkanContext::device.acquireNextImageKHR(
            VulkanContext::swapChain, std::numeric_limits<uint64_t>::max(), vulkanSemaphore->semaphore(), nullptr);
        return result.value;
    } catch (vk::OutOfDateKHRError err) {
        recreateSwapChain();
        return -1;
    }
}

void VulkanInstance::submit(const FenceRef& fence, const SemaphoreRef& waitSemaphore,
    const SemaphoreRef& signalSemaphore, const CommandBufferRef& commandBuffer) const
{
    CHRZONE_VULKAN

    const auto vulkanWaitSemaphore = static_cast<const VulkanSemaphore*>(waitSemaphore.get());
    const auto vulkanSignalSemaphore = static_cast<const VulkanSemaphore*>(signalSemaphore.get());

    const auto vulkanCommandBuffer = static_cast<const VulkanCommandBuffer*>(commandBuffer.get());

    vk::SubmitInfo submitInfo = {};
    std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(vulkanWaitSemaphore->semaphore());
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(vulkanCommandBuffer->commandBuffer());
    submitInfo.setSignalSemaphores(vulkanSignalSemaphore->semaphore());

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());

    VulkanContext::graphicsQueue.submit(submitInfo, vulkanFence->fence());
}

bool VulkanInstance::present(const SemaphoreRef& waitSemaphore, uint32_t imageIndex)
{
    CHRZONE_VULKAN

    const auto vulkanWaitSemaphore = static_cast<const VulkanSemaphore*>(waitSemaphore.get());

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(vulkanWaitSemaphore->semaphore());
    presentInfo.setSwapchains(VulkanContext::swapChain);
    presentInfo.setImageIndices(imageIndex);

    vk::Result resultPresent;
    try {
        resultPresent = VulkanContext::presentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError err) {
        resultPresent = vk::Result::eErrorOutOfDateKHR;
    }

    if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR
        || _swapChainInvalidated) {
        _swapChainInvalidated = false;
        recreateSwapChain();
        return false;
    }

    return true;
}

RendererUnique VulkanInstance::create(App* app) { return std::make_unique<ConcreteVulkanInstance>(app); }

void VulkanInstance::recreateSwapChain()
{
    CHRZONE_VULKAN

    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_app->Window(), &width, &height);
        glfwWaitEvents();
    }

    VulkanContext::device.waitIdle();
    VulkanContext::device.destroySwapchainKHR(VulkanContext::swapChain);

    createSwapChain();
}

void VulkanInstance::createInstance()
{
    CHRZONE_VULKAN

    if (ENABLED_VALIDATION_LAYERS && !checkValidationLayerSupport())
        throw RendererError("Validation layers requested, but not available");

    // application info
    auto appName = _app->AppName();

    vk::ApplicationInfo appInfo = {};
    appInfo.setPApplicationName(appName.c_str());
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setPEngineName("Chronicle");
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setApiVersion(VK_API_VERSION_1_0);

    // create instance info
    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo = {};
    createInfo.setPApplicationInfo(&appInfo);
    createInfo.setPEnabledExtensionNames(extensions);

    if (ENABLED_VALIDATION_LAYERS) {
        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        populateDebugMessengerCreateInfo(debugCreateInfo);

        createInfo.setPEnabledLayerNames(VALIDATION_LAYERS);
        createInfo.setPNext(&debugCreateInfo);
    }

    VulkanContext::instance = vk::createInstance(createInfo, nullptr);
}

void VulkanInstance::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) const
{
    CHRZONE_VULKAN

    using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using enum vk::DebugUtilsMessageTypeFlagBitsEXT;

    createInfo.setMessageSeverity(eVerbose | eInfo | eWarning | eError);
    createInfo.setMessageType(eGeneral | eValidation | ePerformance);
    createInfo.setPfnUserCallback(debugCallback);
}

void VulkanInstance::setupDebugCallback()
{
    CHRZONE_VULKAN

    if (!ENABLED_VALIDATION_LAYERS)
        return;

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    populateDebugMessengerCreateInfo(debugCreateInfo);

    if (createDebugUtilsMessengerEXT(VulkanContext::instance,
            std::bit_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo), nullptr,
            &VulkanContext::debugCallback)
        != VK_SUCCESS)
        throw RendererError("Failed to set up debug callback");
}

void VulkanInstance::createSurface()
{
    CHRZONE_VULKAN

    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(VulkanContext::instance, _app->Window(), nullptr, &rawSurface) != VK_SUCCESS)
        throw RendererError("Failed to create window surface");

    VulkanContext::surface = rawSurface;
}

void VulkanInstance::pickPhysicalDevice()
{
    CHRZONE_VULKAN

    auto devices = VulkanContext::instance.enumeratePhysicalDevices();
    if (devices.size() == 0)
        throw RendererError("Failed to find GPUs with Vulkan support");

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            VulkanContext::physicalDevice = device;
            break;
        }
    }

    if (!VulkanContext::physicalDevice)
        throw RendererError("Failed to find a suitable GPU");
}

void VulkanInstance::createLogicalDevice()
{
    CHRZONE_VULKAN

    VulkanQueueFamilyIndices indices = findQueueFamilies(VulkanContext::physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    std::array<float, 1> queuePriorities = { 1.0f };

    for (auto queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.setQueueFamilyIndex(queueFamily);
        queueCreateInfo.setQueueCount(1);
        queueCreateInfo.setQueuePriorities(queuePriorities);

        queueCreateInfos.push_back(queueCreateInfo);
    }

    auto deviceFeatures = vk::PhysicalDeviceFeatures();
    deviceFeatures.setSamplerAnisotropy(true);

    vk::DeviceCreateInfo createInfo = {};
    createInfo.setQueueCreateInfos(queueCreateInfos);
    createInfo.setPEnabledFeatures(&deviceFeatures);
    createInfo.setPEnabledExtensionNames(DEVICE_EXTENSIONS);

    if (ENABLED_VALIDATION_LAYERS)
        createInfo.setPEnabledLayerNames(VALIDATION_LAYERS);

    VulkanContext::device = VulkanContext::physicalDevice.createDevice(createInfo);

    VulkanContext::graphicsQueue = VulkanContext::device.getQueue(indices.graphicsFamily.value(), 0);
    VulkanContext::presentQueue = VulkanContext::device.getQueue(indices.presentFamily.value(), 0);
}

void VulkanInstance::createSwapChain()
{
    CHRZONE_VULKAN

    auto swapChainSupport = querySwapChainSupport(VulkanContext::physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    auto indices = findQueueFamilies(VulkanContext::physicalDevice);
    std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.setSurface(VulkanContext::surface);
    createInfo.setMinImageCount(imageCount);
    createInfo.setImageFormat(surfaceFormat.format);
    createInfo.setImageColorSpace(surfaceFormat.colorSpace);
    createInfo.setImageExtent(extent);
    createInfo.setImageArrayLayers(1);
    createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform);
    createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    createInfo.setPresentMode(presentMode);
    createInfo.setClipped(true);

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        createInfo.setQueueFamilyIndices(queueFamilyIndices);
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    VulkanContext::swapChain = VulkanContext::device.createSwapchainKHR(createInfo);

    // create depth buffer
    // NOTE: the depth buffer MUST be updated before the swap chain images!
    // the render pass is updated the the swapchain image trigger it, during
    // the framebuffer recreate the depth image must be already updated.
    if (VulkanContext::depthImage) {
        const auto vulkanDepthImage = static_cast<VulkanImage*>(VulkanContext::depthImage.get());
        vulkanDepthImage->updateDepthBuffer(extent.width, extent.height, VulkanContext::depthImageFormat);
    } else {
        VulkanContext::depthImageFormat = findDepthFormat();
        VulkanContext::depthImage
            = VulkanImage::createDepthBuffer(extent.width, extent.height, VulkanContext::depthImageFormat);
    }

    // create swap chain images
    auto swapChainImages = VulkanContext::device.getSwapchainImagesKHR(VulkanContext::swapChain);

    VulkanContext::swapChainImages.reserve(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        if (VulkanContext::swapChainImages.size() > i) {
            const auto vulkanImage = static_cast<VulkanImage*>(VulkanContext::swapChainImages[i].get());
            vulkanImage->updateSwapchain(swapChainImages[i], surfaceFormat.format, extent.width, extent.height);
        } else {
            auto image
                = VulkanImage::createSwapchain(swapChainImages[i], surfaceFormat.format, extent.width, extent.height);
            VulkanContext::swapChainImages.push_back(image);
        }
    }

    VulkanContext::swapChainImageFormat = surfaceFormat.format;
    VulkanContext::swapChainExtent = extent;
}

void VulkanInstance::createCommandPool()
{
    CHRZONE_VULKAN

    auto queueFamilyIndices = findQueueFamilies(VulkanContext::physicalDevice);
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

    VulkanContext::commandPool = VulkanContext::device.createCommandPool(poolInfo);
}

bool VulkanInstance::checkValidationLayerSupport() const
{
    CHRZONE_VULKAN

    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : VALIDATION_LAYERS) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

std::vector<const char*> VulkanInstance::getRequiredExtensions() const
{
    CHRZONE_VULKAN

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLED_VALIDATION_LAYERS)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool VulkanInstance::isDeviceSuitable(const vk::PhysicalDevice& physicalDevice) const
{
    CHRZONE_VULKAN

    VulkanQueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    const auto supportedFeatures = physicalDevice.getFeatures();
    return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanInstance::checkDeviceExtensionSupport(const vk::PhysicalDevice& device) const
{
    CHRZONE_VULKAN

    std::set<std::string, std::less<>> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto& extension : device.enumerateDeviceExtensionProperties())
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

VulkanQueueFamilyIndices VulkanInstance::findQueueFamilies(vk::PhysicalDevice device) const
{
    CHRZONE_VULKAN

    VulkanQueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, VulkanContext::surface))
            indices.presentFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

VulkanSwapChainSupportDetails VulkanInstance::querySwapChainSupport(const vk::PhysicalDevice& device) const
{
    CHRZONE_VULKAN

    VulkanSwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(VulkanContext::surface);
    details.formats = device.getSurfaceFormatsKHR(VulkanContext::surface);
    details.presentModes = device.getSurfacePresentModesKHR(VulkanContext::surface);
    return details;
}

vk::SurfaceFormatKHR VulkanInstance::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
    CHRZONE_VULKAN

    using enum vk::Format;

    if (availableFormats.size() == 1 && availableFormats[0].format == eUndefined)
        return { eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    }

    return availableFormats[0];
}

vk::PresentModeKHR VulkanInstance::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) const
{
    CHRZONE_VULKAN

    using enum vk::PresentModeKHR;

    vk::PresentModeKHR bestMode = eFifo;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == eMailbox)
            return availablePresentMode;
        else if (availablePresentMode == eImmediate)
            bestMode = availablePresentMode;
    }

    return bestMode;
}

vk::Extent2D VulkanInstance::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
    CHRZONE_VULKAN

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width;
        int height;
        glfwGetFramebufferSize(_app->Window(), &width, &height);

        vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        actualExtent.width = std::max(
            capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

vk::Format VulkanInstance::findSupportedFormat(
    const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const
{
    for (const auto& format : candidates) {
        auto props = VulkanContext::physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw RendererError("Failed to find supported format");
}

vk::Format VulkanInstance::findDepthFormat() const
{
    return findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool VulkanInstance::hasStencilComponent(vk::Format format) const
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

} // namespace chronicle