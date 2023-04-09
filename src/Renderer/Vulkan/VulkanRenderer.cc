#include "VulkanRenderer.h"

#include "Platform/App.h"

#include "VulkanCommandBuffer.h"
#include "VulkanFence.h"
#include "VulkanImage.h"
#include "VulkanRenderer.h"
#include "VulkanSemaphore.h"

const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool ENABLED_VALIDATION_LAYERS = false;
#else
const bool ENABLED_VALIDATION_LAYERS = true;
#endif

namespace chronicle {

CHR_CONCRETE(VulkanRenderer)

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

VulkanRenderer::VulkanRenderer(chronicle::App* app)
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

VulkanRenderer::~VulkanRenderer()
{
    CHRZONE_VULKAN

    _depthImage.reset();
    _swapChainImages.clear();

    _device.destroySwapchainKHR(_swapChain);
    _device.destroyCommandPool(_commandPool);
    _device.destroy();

    if (ENABLED_VALIDATION_LAYERS) {
        destroyDebugUtilsMessengerEXT(_instance, _debugCallback, nullptr);
    }

    _instance.destroySurfaceKHR(_surface);
    _instance.destroy();
}

void VulkanRenderer::waitIdle() const
{
    CHRZONE_VULKAN

    _device.waitIdle();
}

void VulkanRenderer::waitForFence(const FenceRef& fence) const
{
    CHRZONE_VULKAN

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());

    (void)_device.waitForFences(vulkanFence->fence(), true, std::numeric_limits<uint64_t>::max());
}

void VulkanRenderer::resetFence(const FenceRef& fence) const
{
    CHRZONE_VULKAN

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());

    (void)_device.resetFences(vulkanFence->fence());
}

uint32_t VulkanRenderer::acquireNextImage(const SemaphoreRef& semaphore)
{
    CHRZONE_VULKAN

    const auto vulkanSemaphore = static_cast<const VulkanSemaphore*>(semaphore.get());

    try {
        auto result = _device.acquireNextImageKHR(
            _swapChain, std::numeric_limits<uint64_t>::max(), vulkanSemaphore->semaphore(), nullptr);
        return result.value;
    } catch (vk::OutOfDateKHRError err) {
        recreateSwapChain();
        return -1;
    }
}

void VulkanRenderer::submit(const FenceRef& fence, const SemaphoreRef& waitSemaphore,
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

    _graphicsQueue.submit(submitInfo, vulkanFence->fence());
}

bool VulkanRenderer::present(const SemaphoreRef& waitSemaphore, uint32_t imageIndex)
{
    CHRZONE_VULKAN

    const auto vulkanWaitSemaphore = static_cast<const VulkanSemaphore*>(waitSemaphore.get());

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(vulkanWaitSemaphore->semaphore());
    presentInfo.setSwapchains(_swapChain);
    presentInfo.setImageIndices(imageIndex);

    vk::Result resultPresent;
    try {
        resultPresent = _presentQueue.presentKHR(presentInfo);
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

RendererUnique VulkanRenderer::create(App* app) { return std::make_unique<ConcreteVulkanRenderer>(app); }

void VulkanRenderer::recreateSwapChain()
{
    CHRZONE_VULKAN

    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_app->Window(), &width, &height);
        glfwWaitEvents();
    }

    _device.waitIdle();
    _device.destroySwapchainKHR(_swapChain);

    createSwapChain();
}

void VulkanRenderer::createInstance()
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

    _instance = vk::createInstance(createInfo, nullptr);
}

void VulkanRenderer::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) const
{
    CHRZONE_VULKAN

    using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using enum vk::DebugUtilsMessageTypeFlagBitsEXT;

    createInfo.setMessageSeverity(eVerbose | eInfo | eWarning | eError);
    createInfo.setMessageType(eGeneral | eValidation | ePerformance);
    createInfo.setPfnUserCallback(debugCallback);
}

void VulkanRenderer::setupDebugCallback()
{
    CHRZONE_VULKAN

    if (!ENABLED_VALIDATION_LAYERS)
        return;

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    populateDebugMessengerCreateInfo(debugCreateInfo);

    if (createDebugUtilsMessengerEXT(_instance,
            std::bit_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo), nullptr, &_debugCallback)
        != VK_SUCCESS)
        throw RendererError("Failed to set up debug callback");
}

void VulkanRenderer::createSurface()
{
    CHRZONE_VULKAN

    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(_instance, _app->Window(), nullptr, &rawSurface) != VK_SUCCESS)
        throw RendererError("Failed to create window surface");

    _surface = rawSurface;
}

void VulkanRenderer::pickPhysicalDevice()
{
    CHRZONE_VULKAN

    auto devices = _instance.enumeratePhysicalDevices();
    if (devices.size() == 0)
        throw RendererError("Failed to find GPUs with Vulkan support");

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            _physicalDevice = device;
            break;
        }
    }

    if (!_physicalDevice)
        throw RendererError("Failed to find a suitable GPU");
}

void VulkanRenderer::createLogicalDevice()
{
    CHRZONE_VULKAN

    VulkanQueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

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

    _device = _physicalDevice.createDevice(createInfo);

    _graphicsQueue = _device.getQueue(indices.graphicsFamily.value(), 0);
    _presentQueue = _device.getQueue(indices.presentFamily.value(), 0);
}

void VulkanRenderer::createSwapChain()
{
    CHRZONE_VULKAN

    auto swapChainSupport = querySwapChainSupport(_physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    auto indices = findQueueFamilies(_physicalDevice);
    std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.setSurface(_surface);
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

    _swapChain = _device.createSwapchainKHR(createInfo);

    // create depth buffer
    // NOTE: the depth buffer MUST be updated before the swap chain images!
    // the render pass is updated the the swapchain image trigger it, during
    // the framebuffer recreate the depth image must be already updated.
    if (_depthImage) {
        const auto vulkanDepthImage = static_cast<VulkanImage*>(_depthImage.get());
        vulkanDepthImage->updateDepthBuffer(extent.width, extent.height, _depthImageFormat);
    } else {
        _depthImageFormat = findDepthFormat();
        _depthImage
            = VulkanImage::createDepthBuffer(_device, _physicalDevice, extent.width, extent.height, _depthImageFormat);
    }

    // create swap chain images
    auto swapChainImages = _device.getSwapchainImagesKHR(_swapChain);

    _swapChainImages.reserve(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        if (_swapChainImages.size() > i) {
            const auto vulkanImage = static_cast<VulkanImage*>(_swapChainImages[i].get());
            vulkanImage->updateSwapchain(swapChainImages[i], surfaceFormat.format, extent.width, extent.height);
        } else {
            auto image = VulkanImage::createSwapchain(
                _device, swapChainImages[i], surfaceFormat.format, extent.width, extent.height);
            _swapChainImages.push_back(image);
        }
    }

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void VulkanRenderer::createCommandPool()
{
    CHRZONE_VULKAN

    auto queueFamilyIndices = findQueueFamilies(_physicalDevice);
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

    _commandPool = _device.createCommandPool(poolInfo);
}

bool VulkanRenderer::checkValidationLayerSupport() const
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

std::vector<const char*> VulkanRenderer::getRequiredExtensions() const
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

bool VulkanRenderer::isDeviceSuitable(const vk::PhysicalDevice& physicalDevice) const
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

bool VulkanRenderer::checkDeviceExtensionSupport(const vk::PhysicalDevice& device) const
{
    CHRZONE_VULKAN

    std::set<std::string, std::less<>> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto& extension : device.enumerateDeviceExtensionProperties())
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

VulkanQueueFamilyIndices VulkanRenderer::findQueueFamilies(vk::PhysicalDevice device) const
{
    CHRZONE_VULKAN

    VulkanQueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, _surface))
            indices.presentFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

VulkanSwapChainSupportDetails VulkanRenderer::querySwapChainSupport(const vk::PhysicalDevice& device) const
{
    CHRZONE_VULKAN

    VulkanSwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(_surface);
    details.formats = device.getSurfaceFormatsKHR(_surface);
    details.presentModes = device.getSurfacePresentModesKHR(_surface);
    return details;
}

vk::SurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(
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

vk::PresentModeKHR VulkanRenderer::chooseSwapPresentMode(
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

vk::Extent2D VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
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

vk::Format VulkanRenderer::findSupportedFormat(
    const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const
{
    for (const auto& format : candidates) {
        auto props = _physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw RendererError("Failed to find supported format");
}

vk::Format VulkanRenderer::findDepthFormat() const
{
    return findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool VulkanRenderer::hasStencilComponent(vk::Format format) const
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

} // namespace chronicle