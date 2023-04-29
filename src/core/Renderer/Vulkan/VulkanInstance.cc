// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanInstance.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanExtensions.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"

#ifdef GLFW_PLATFORM
#include "Platform/GLFW/GLFWCommon.h"
#endif

using namespace entt::literals;

namespace chronicle {

const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME
//#ifdef VULKAN_ENABLE_DEBUG_MARKER
//    ,
//    VK_EXT_DEBUG_MARKER_EXTENSION_NAME
//#endif // VULKAN_ENABLE_DEBUG_MARKER
};

CHR_CONCRETE(VulkanInstance);

/// @brief Debug messages callback.
/// @param messageSeverity Bitmask specifying which severities of events cause a debug messenger callback.
/// @param messageType Bitmask specifying which types of events cause a debug messenger callback.
/// @param pCallbackData Structure specifying parameters returned to the callback.
/// @param pUserData User opaque data.
/// @return 0 if success otherwise error code.
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CHRLOG_ERROR("{}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        CHRLOG_WARN("{}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        CHRLOG_INFO("{}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        CHRLOG_DEBUG("{}", pCallbackData->pMessage);
    }
    return 0;
}

void VulkanInstance::init()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Vulkan instance init");

    // allocate data for frame in flights
    VulkanContext::framesData.resize(VulkanContext::maxFramesInFlight);

    // initialize everything
    createInstance();
    setupDebugCallback();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createCommandPool();
    createRenderPass();
    createDebugRenderPass();
    createFramebuffers();
    createDebugFramebuffers();
    createSyncObjects();
    createCommandBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void VulkanInstance::deinit()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Vulkan instance deinit");

    // reset descriptor sets references
    // this must be happen before to clean the garbage collector
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        VulkanContext::framesData[i].descriptorSet.reset();
    }

    // clean data from frames garbage collectors
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        VulkanUtils::cleanupGarbageCollector(VulkanContext::framesData[i].garbageCollector);
    }

    // wait for garbage collector destruction
    VulkanContext::device.waitIdle();

    // destroy destriptor pool
    VulkanContext::device.destroyDescriptorPool(VulkanContext::descriptorPool);

    // clean frame data
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        // destroy synchronization objects
        VulkanContext::device.destroySemaphore(VulkanContext::framesData[i].imageAvailableSemaphore);
        VulkanContext::device.destroySemaphore(VulkanContext::framesData[i].renderFinishedSemaphore);
        VulkanContext::device.destroyFence(VulkanContext::framesData[i].inFlightFence);
    }

    VulkanContext::framesData.clear();

    // destroy rendering passes
    VulkanContext::device.destroyRenderPass(VulkanContext::debugRenderPass);
    VulkanContext::device.destroyRenderPass(VulkanContext::renderPass);

    // cleanup swap chain
    cleanupSwapChain();

    // destroy command pool
    VulkanContext::device.destroyCommandPool(VulkanContext::commandPool);

    // destroy device
    VulkanContext::device.destroy();

    // destroy debug utilities if registered
    if (VulkanContext::enabledValidationLayer) {
        destroyDebugUtilsMessengerEXT(VulkanContext::instance, VulkanContext::debugCallback, nullptr);
    }

    // destroy surface
    VulkanContext::instance.destroySurfaceKHR(VulkanContext::surface);

    // destroy instance
    VulkanContext::instance.destroy();
}

void VulkanInstance::recreateSwapChain()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Recreate swapchain");

    int width = 0;
    int height = 0;
#ifdef GLFW_PLATFORM
    // get the new frambuffer size (try indefintely if minimezed)
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(GLFWContext::window, &width, &height);
        glfwWaitEvents();
    }
#else
    throw RendererError("Not implemented");
#endif

    // wait for GPU idle
    VulkanContext::device.waitIdle();

    // clean up old swapchain
    cleanupSwapChain();

    // create new swapchain
    createSwapChain();

    // create main and debug framebuffers
    createFramebuffers();
    createDebugFramebuffers();
}

void VulkanInstance::cleanupSwapChain()
{
    CHRZONE_RENDERER;

    // clean framebuffers
    for (const auto& imageData : VulkanContext::imagesData) {
        VulkanContext::device.destroyFramebuffer(imageData.framebuffer);
        VulkanContext::device.destroyFramebuffer(imageData.debugFramebuffer);
    }

    // clean color images and images view
    VulkanContext::device.destroyImageView(VulkanContext::colorImageView);
    VulkanContext::device.destroyImage(VulkanContext::colorImage);
    VulkanContext::device.freeMemory(VulkanContext::colorImageMemory);

    // clean depth images and images view
    VulkanContext::device.destroyImageView(VulkanContext::depthImageView);
    VulkanContext::device.destroyImage(VulkanContext::depthImage);
    VulkanContext::device.freeMemory(VulkanContext::depthImageMemory);

    // clean swap chain image views
    for (const auto& imageData : VulkanContext::imagesData) {
        VulkanContext::device.destroyImageView(imageData.swapChainImageView);
    }

    // clean image data
    VulkanContext::imagesData.clear();

    // destroy swapchain
    VulkanContext::device.destroySwapchainKHR(VulkanContext::swapChain);
}

void VulkanInstance::createInstance()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create instance");

    // check validation layers support
    if (VulkanContext::enabledValidationLayer && !VulkanUtils::checkValidationLayerSupport(VALIDATION_LAYERS))
        throw RendererError("Validation layers requested, but not available");

    // application info
    vk::ApplicationInfo appInfo = {};
    appInfo.setPApplicationName("Chronicle");
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setPEngineName("Chronicle");
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setApiVersion(VK_API_VERSION_1_0);

    // prepare create instance info
    vk::InstanceCreateInfo createInfo = {};
    createInfo.setPApplicationInfo(&appInfo);
    auto extensions = VulkanUtils::getRequiredExtensions();
    createInfo.setPEnabledExtensionNames(extensions);

    // add debug related stuff if validation layers are enabled
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (VulkanContext::enabledValidationLayer) {
        populateDebugMessengerCreateInfo(debugCreateInfo);

        createInfo.setPEnabledLayerNames(VALIDATION_LAYERS);
        createInfo.setPNext(&debugCreateInfo);
    }

    // create the instance
    VulkanContext::instance = vk::createInstance(createInfo, nullptr);
}

void VulkanInstance::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
    using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using enum vk::DebugUtilsMessageTypeFlagBitsEXT;

    // populate the data structure
    createInfo.setMessageSeverity(eVerbose | eInfo | eWarning | eError);
    createInfo.setMessageType(eGeneral | eValidation | ePerformance);
    createInfo.setPfnUserCallback(debugCallback);
}

void VulkanInstance::setupDebugCallback()
{
    CHRZONE_RENDERER;

    // skip if validation layers are not enabled
    if (!VulkanContext::enabledValidationLayer)
        return;

    // create the debug utils messenger
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
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create surface");

#ifdef GLFW_PLATFORM
    // create the surface
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(VulkanContext::instance, GLFWContext::window, nullptr, &rawSurface) != VK_SUCCESS)
        throw RendererError("Failed to create window surface");
    VulkanContext::surface = rawSurface;
#else
    throw RendererError("Not implemented");
#endif
}

void VulkanInstance::pickPhysicalDevice()
{
    CHRZONE_RENDERER;

    // enumerate all physical devices
    auto devices = VulkanContext::instance.enumeratePhysicalDevices();
    if (devices.size() == 0)
        throw RendererError("Failed to find GPUs with Vulkan support");

    // iterate all devices and get the first suitable one
    for (const auto& device : devices) {
        if (VulkanUtils::isDeviceSuitable(device, DEVICE_EXTENSIONS)) {
            VulkanContext::physicalDevice = device;
            VulkanContext::msaaSamples = VulkanUtils::getMaxUsableSampleCount();
            break;
        }
    }

    // check if a physical device where found
    if (!VulkanContext::physicalDevice)
        throw RendererError("Failed to find a suitable GPU");
}

void VulkanInstance::createLogicalDevice()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create logical device");

    // find device queue families
    VulkanQueueFamilyIndices indices = VulkanUtils::findQueueFamilies(VulkanContext::physicalDevice);

    // prepare the device create info for every family
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

    // get and enabled the device features
    auto deviceFeatures = vk::PhysicalDeviceFeatures();
    deviceFeatures.setSamplerAnisotropy(true);
    deviceFeatures.setFillModeNonSolid(true);

    // create the logical device
    vk::DeviceCreateInfo createInfo = {};
    createInfo.setQueueCreateInfos(queueCreateInfos);
    createInfo.setPEnabledFeatures(&deviceFeatures);
    createInfo.setPEnabledExtensionNames(DEVICE_EXTENSIONS);
    if (VulkanContext::enabledValidationLayer)
        createInfo.setPEnabledLayerNames(VALIDATION_LAYERS);
    VulkanContext::device = VulkanContext::physicalDevice.createDevice(createInfo);

    // store graphics queue and families
    VulkanContext::graphicsQueue = VulkanContext::device.getQueue(indices.graphicsFamily.value(), 0);
    VulkanContext::presentQueue = VulkanContext::device.getQueue(indices.presentFamily.value(), 0);
    VulkanContext::graphicsFamily = indices.graphicsFamily.value();
    VulkanContext::presentFamily = indices.presentFamily.value();
}

void VulkanInstance::createSwapChain()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create swapchain");

    // get all the informations required for create the swapchain
    auto swapChainSupport = VulkanUtils::querySwapChainSupport(VulkanContext::physicalDevice);
    auto surfaceFormat = VulkanUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = VulkanUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = VulkanUtils::chooseSwapExtent(swapChainSupport.capabilities);
    auto indices = VulkanUtils::findQueueFamilies(VulkanContext::physicalDevice);

    // calculate the image count
    auto imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    // create the swapchain
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
    std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        createInfo.setQueueFamilyIndices(queueFamilyIndices);
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }
    VulkanContext::swapChain = VulkanContext::device.createSwapchainKHR(createInfo);

    // create color buffer
    auto [colorImageMemory, colorImage] = VulkanUtils::createImage(extent.width, extent.height, 1,
        VulkanContext::msaaSamples, surfaceFormat.format, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    VulkanContext::colorImage = colorImage;
    VulkanContext::colorImageMemory = colorImageMemory;
    VulkanContext::colorImageView
        = VulkanUtils::createImageView(colorImage, surfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1);

    // create depth buffer
    VulkanContext::depthImageFormat = VulkanUtils::findDepthFormat();
    auto [depthImageMemory, depthImage] = VulkanUtils::createImage(extent.width, extent.height, 1,
        VulkanContext::msaaSamples, VulkanContext::depthImageFormat, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    VulkanContext::depthImageMemory = depthImageMemory;
    VulkanContext::depthImage = depthImage;
    VulkanContext::depthImageView
        = VulkanUtils::createImageView(depthImage, VulkanContext::depthImageFormat, vk::ImageAspectFlagBits::eDepth, 1);

    // create swap chain images
    auto swapChainImages = VulkanContext::device.getSwapchainImagesKHR(VulkanContext::swapChain);
    VulkanContext::swapChainImageFormat = surfaceFormat.format;
    VulkanContext::swapChainExtent = extent;

    // prepare images data structures
    VulkanContext::imagesData.resize(swapChainImages.size());
    for (auto i = 0; i < swapChainImages.size(); i++) {
        VulkanContext::imagesData[i].swapChainImage = swapChainImages[i];
        VulkanContext::imagesData[i].swapChainImageView = VulkanUtils::createImageView(
            swapChainImages[i], surfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1);
    }
}

void VulkanInstance::createCommandPool()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create command pool");

    // get the queue families
    auto queueFamilyIndices = VulkanUtils::findQueueFamilies(VulkanContext::physicalDevice);

    // create the command pool
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());
    VulkanContext::commandPool = VulkanContext::device.createCommandPool(poolInfo);
}

void VulkanInstance::createRenderPass()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create main render pass");

    // color attachment
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.setFormat(VulkanContext::swapChainImageFormat);
    colorAttachment.setSamples(VulkanContext::msaaSamples);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    colorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // color attachment reference
    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.setAttachment(0);
    colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // depth attachment
    vk::AttachmentDescription depthAttachment = {};
    depthAttachment.setFormat(VulkanContext::depthImageFormat);
    depthAttachment.setSamples(VulkanContext::msaaSamples);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    // depth attachment reference
    vk::AttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.setAttachment(1);
    depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    // resolve attachment
    vk::AttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.setFormat(VulkanContext::swapChainImageFormat);
    colorAttachmentResolve.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachmentResolve.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachmentResolve.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachmentResolve.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachmentResolve.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachmentResolve.setInitialLayout(vk::ImageLayout::eUndefined);
    colorAttachmentResolve.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    // resolve attachment reference
    vk::AttachmentReference colorAttachmentResolveRef = {};
    colorAttachmentResolveRef.setAttachment(2);
    colorAttachmentResolveRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // subpass
    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachments(colorAttachmentRef);
    subpass.setPDepthStencilAttachment(&depthAttachmentRef);
    subpass.setPResolveAttachments(&colorAttachmentResolveRef);

    // dependency
    vk::SubpassDependency dependency = {};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setDstSubpass(0);
    dependency.setSrcStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    dependency.setDstStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    dependency.setDstAccessMask(
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    // create the renderpass
    vk::RenderPassCreateInfo createRenderPassInfo = {};
    std::array<vk::AttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    createRenderPassInfo.setAttachments(attachments);
    createRenderPassInfo.setSubpasses(subpass);
    createRenderPassInfo.setDependencies(dependency);
    VulkanContext::renderPass = VulkanContext::device.createRenderPass(createRenderPassInfo);
}

void VulkanInstance::createDebugRenderPass()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create ImGui render pass");

    // color attachment
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.setFormat(VulkanContext::swapChainImageFormat);
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eLoad);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::ePresentSrcKHR);
    colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    // color attachment reference
    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.setAttachment(0);
    colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // subpass
    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachments(colorAttachmentRef);

    // dependency
    vk::SubpassDependency dependency = {};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setDstSubpass(0);
    dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    // create the renderpass
    vk::RenderPassCreateInfo createRenderPassInfo = {};
    std::array<vk::AttachmentDescription, 1> attachments = { colorAttachment };
    createRenderPassInfo.setAttachments(attachments);
    createRenderPassInfo.setSubpasses(subpass);
    createRenderPassInfo.setDependencies(dependency);
    VulkanContext::debugRenderPass = VulkanContext::device.createRenderPass(createRenderPassInfo);
}

void VulkanInstance::createFramebuffers()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create framebuffers");

    // create the framebuffers
    for (auto& imageData : VulkanContext::imagesData) {
        std::array<vk::ImageView, 3> attachments
            = { VulkanContext::colorImageView, VulkanContext::depthImageView, imageData.swapChainImageView };
        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.setRenderPass(VulkanContext::renderPass);
        framebufferInfo.setAttachments(attachments);
        framebufferInfo.setWidth(VulkanContext::swapChainExtent.width);
        framebufferInfo.setHeight(VulkanContext::swapChainExtent.height);
        framebufferInfo.setLayers(1);
        imageData.framebuffer = VulkanContext::device.createFramebuffer(framebufferInfo);
    }
}

void VulkanInstance::createDebugFramebuffers()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create ImGui framebuffers");

    // create the framebuffers
    for (auto& imageData : VulkanContext::imagesData) {
        std::array<vk::ImageView, 1> attachments = { imageData.swapChainImageView };
        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.setRenderPass(VulkanContext::debugRenderPass);
        framebufferInfo.setAttachments(attachments);
        framebufferInfo.setWidth(VulkanContext::swapChainExtent.width);
        framebufferInfo.setHeight(VulkanContext::swapChainExtent.height);
        framebufferInfo.setLayers(1);
        imageData.debugFramebuffer = VulkanContext::device.createFramebuffer(framebufferInfo);
    }
}

void VulkanInstance::createSyncObjects()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create sync objects");

    // create synchronization objects
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        VulkanContext::framesData[i].imageAvailableSemaphore = VulkanContext::device.createSemaphore({});
        VulkanContext::framesData[i].renderFinishedSemaphore = VulkanContext::device.createSemaphore({});
        VulkanContext::framesData[i].inFlightFence
            = VulkanContext::device.createFence({ vk::FenceCreateFlagBits::eSignaled });
    }
}

void VulkanInstance::createCommandBuffers()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create command buffers");

    // create the command buffers
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
#ifdef VULKAN_ENABLE_DEBUG_MARKER
        auto tmpDebugName = std::format("Main command buffer (frame {})", i);
        const char* debugName = tmpDebugName.c_str();
#else
        const char* debugName = nullptr;
#endif // VULKAN_ENABLE_DEBUG_MARKER

        VulkanContext::framesData[i].commandBuffer = VulkanCommandBuffer::create(debugName);
    }
}

void VulkanInstance::createDescriptorSets()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create descriptor sets");

    // create the descriptor sets
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
#ifdef VULKAN_ENABLE_DEBUG_MARKER
        auto tmpDebugName = std::format("Main descriptor set (frame {})", i);
        const char* debugName = tmpDebugName.c_str();
#else
        const char* debugName = nullptr;
#endif // VULKAN_ENABLE_DEBUG_MARKER

        auto descriptorSet = DescriptorSet::create(debugName);
        descriptorSet->addUniform<UniformBufferObject>("ubo"_hs, ShaderStage::Vertex);
        VulkanContext::framesData[i].descriptorSet = descriptorSet;
    }
}

void VulkanInstance::createDescriptorPool()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create descriptor pool");

    // some default sizes for the pool
    std::vector<vk::DescriptorPoolSize> sizes
        = { { vk::DescriptorType::eUniformBuffer, 1000 }, { vk::DescriptorType::eCombinedImageSampler, 1000 } };

    // create the pool
    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.setMaxSets(static_cast<uint32_t>(10000 * sizes.size()));
    poolInfo.setPoolSizes(sizes);
    VulkanContext::descriptorPool = VulkanContext::device.createDescriptorPool(poolInfo, nullptr);
}

} // namespace chronicle