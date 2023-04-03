#include "VulkanRenderer.h"

#include "Platform/App.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/Fence.h"
#include "Renderer/Image.h"
#include "Renderer/RendererError.h"
#include "Renderer/Semaphore.h"

const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool ENABLED_VALIDATION_LAYERS = false;
#else
const bool ENABLED_VALIDATION_LAYERS = true;
#endif

namespace chronicle {

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
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        spdlog::error("{}", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        spdlog::warn("{}", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        spdlog::info("{}", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        spdlog::debug("{}", pCallbackData->pMessage);
    return 0;
}

VulkanRenderer::VulkanRenderer(chronicle::App* app)
    : _app(app)
{
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

void VulkanRenderer::waitIdle() const { _device.waitIdle(); }

void VulkanRenderer::waitForFence(const std::shared_ptr<Fence>& fence) const
{
    (void)_device.waitForFences(fence->native().fence(), true, std::numeric_limits<uint64_t>::max());
}

void VulkanRenderer::resetFence(const std::shared_ptr<Fence>& fence) const
{
    (void)_device.resetFences(fence->native().fence());
}

uint32_t VulkanRenderer::acquireNextImage(const std::shared_ptr<Semaphore>& semaphore)
{
    try {
        auto result = _device.acquireNextImageKHR(
            _swapChain, std::numeric_limits<uint64_t>::max(), semaphore->native().semaphore(), nullptr);
        return result.value;
    } catch (vk::OutOfDateKHRError err) {
        recreateSwapChain();
        return -1;
    }
}

void VulkanRenderer::submit(const std::shared_ptr<Fence>& fence, const std::shared_ptr<Semaphore>& waitSemaphore,
    const std::shared_ptr<Semaphore>& signalSemaphore, const std::shared_ptr<CommandBuffer>& commandBuffer) const
{
    vk::SubmitInfo submitInfo = {};
    std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(waitSemaphore->native().semaphore());
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(commandBuffer->native().commandBuffer());
    submitInfo.setSignalSemaphores(signalSemaphore->native().semaphore());

    _graphicsQueue.submit(submitInfo, fence->native().fence());
}

bool VulkanRenderer::present(const std::shared_ptr<Semaphore>& waitSemaphore, uint32_t imageIndex)
{
    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(waitSemaphore->native().semaphore());
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

void VulkanRenderer::recreateSwapChain()
{
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
    createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    createInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

    createInfo.setPfnUserCallback(debugCallback);
}

void VulkanRenderer::setupDebugCallback()
{
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
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(_instance, _app->Window(), nullptr, &rawSurface) != VK_SUCCESS)
        throw RendererError("Failed to create window surface");

    _surface = rawSurface;
}

void VulkanRenderer::pickPhysicalDevice()
{
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

    auto swapChainImages = _device.getSwapchainImagesKHR(_swapChain);

    _swapChainImages.reserve(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        using enum vk::ComponentSwizzle;

        vk::ComponentMapping components = {};
        components.setR(eIdentity);
        components.setG(eIdentity);
        components.setB(eIdentity);
        components.setA(eIdentity);

        vk::ImageSubresourceRange subresourceRange = {};
        subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        subresourceRange.setBaseMipLevel(0);
        subresourceRange.setLevelCount(1);
        subresourceRange.setBaseArrayLayer(0);
        subresourceRange.setLayerCount(1);

        vk::ImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.setImage(swapChainImages[i]);
        imageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
        imageViewCreateInfo.setFormat(surfaceFormat.format);
        imageViewCreateInfo.setComponents(components);
        imageViewCreateInfo.setSubresourceRange(subresourceRange);

        auto imageView = _device.createImageView(imageViewCreateInfo);
        if (_swapChainImages.size() > i) {
            _swapChainImages[i]->native().updateImage(swapChainImages[i], imageView, extent.width, extent.height);
        } else {
            auto image = std::make_shared<Image>(_device, swapChainImages[i], imageView, extent.width, extent.height);
            _swapChainImages.push_back(image);
        }
    }

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void VulkanRenderer::createCommandPool()
{
    auto queueFamilyIndices = findQueueFamilies(_physicalDevice);
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

    _commandPool = _device.createCommandPool(poolInfo);
}

bool VulkanRenderer::checkValidationLayerSupport() const
{
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
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLED_VALIDATION_LAYERS)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool VulkanRenderer::isDeviceSuitable(const vk::PhysicalDevice& device) const
{
    VulkanQueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanRenderer::checkDeviceExtensionSupport(const vk::PhysicalDevice& device) const
{
    std::set<std::string, std::less<>> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto& extension : device.enumerateDeviceExtensionProperties())
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

VulkanQueueFamilyIndices VulkanRenderer::findQueueFamilies(vk::PhysicalDevice device) const
{
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
    VulkanSwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(_surface);
    details.formats = device.getSurfaceFormatsKHR(_surface);
    details.presentModes = device.getSurfacePresentModesKHR(_surface);
    return details;
}

vk::SurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
        return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm
            && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    }

    return availableFormats[0];
}

vk::PresentModeKHR VulkanRenderer::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) const
{
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
            return availablePresentMode;
        else if (availablePresentMode == vk::PresentModeKHR::eImmediate)
            bestMode = availablePresentMode;
    }

    return bestMode;
}

vk::Extent2D VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
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

} // namespace chronicle