// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanInstance.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"

#ifdef GLFW_PLATFORM
#include "Platform/GLFW/GLFWCommon.h"
#endif

using namespace entt::literals;

namespace chronicle {

const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

CHR_CONCRETE(VulkanInstance);

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

    VulkanContext::descriptorSets.clear();
    VulkanContext::device.destroyDescriptorPool(VulkanContext::descriptorPool);

    VulkanContext::commandBuffers.clear();

    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        VulkanContext::device.destroySemaphore(VulkanContext::imageAvailableSemaphores[i]);
        VulkanContext::device.destroySemaphore(VulkanContext::renderFinishedSemaphores[i]);
        VulkanContext::device.destroyFence(VulkanContext::inFlightFences[i]);
    }
    VulkanContext::imageAvailableSemaphores.clear();
    VulkanContext::renderFinishedSemaphores.clear();
    VulkanContext::inFlightFences.clear();

    VulkanContext::device.destroyRenderPass(VulkanContext::debugRenderPass);
    VulkanContext::device.destroyRenderPass(VulkanContext::renderPass);

    cleanupSwapChain();

    VulkanContext::device.destroyCommandPool(VulkanContext::commandPool);
    VulkanContext::device.destroy();

    if (VulkanContext::enabledValidationLayer) {
        destroyDebugUtilsMessengerEXT(VulkanContext::instance, VulkanContext::debugCallback, nullptr);
    }

    VulkanContext::instance.destroySurfaceKHR(VulkanContext::surface);
    VulkanContext::instance.destroy();
}

void VulkanInstance::recreateSwapChain()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Recreate swapchain");

#ifdef GLFW_PLATFORM
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(GLFWContext::window, &width, &height);
        glfwWaitEvents();
    }
#endif

    VulkanContext::device.waitIdle();

    cleanupSwapChain();
    createSwapChain();
    createFramebuffers();
    createDebugFramebuffers();
}

void VulkanInstance::cleanupSwapChain()
{
    CHRZONE_RENDERER;

    // clean debug framebuffers
    for (const auto& framebuffer : VulkanContext::debugFramebuffers) {
        VulkanContext::device.destroyFramebuffer(framebuffer);
    }
    VulkanContext::debugFramebuffers.clear();

    // clean main frame buffers
    for (const auto& framebuffer : VulkanContext::framebuffers) {
        VulkanContext::device.destroyFramebuffer(framebuffer);
    }
    VulkanContext::framebuffers.clear();

    // clean images and images view
    VulkanContext::device.destroyImageView(VulkanContext::colorImageView);
    VulkanContext::device.destroyImage(VulkanContext::colorImage);
    VulkanContext::device.freeMemory(VulkanContext::colorImageMemory);

    VulkanContext::device.destroyImageView(VulkanContext::depthImageView);
    VulkanContext::device.destroyImage(VulkanContext::depthImage);
    VulkanContext::device.freeMemory(VulkanContext::depthImageMemory);

    for (const auto& imageView : VulkanContext::swapChainImageViews) {
        VulkanContext::device.destroyImageView(imageView);
    }
    VulkanContext::swapChainImageViews.clear();

    VulkanContext::device.destroySwapchainKHR(VulkanContext::swapChain);
}

void VulkanInstance::createInstance()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create instance");

    if (VulkanContext::enabledValidationLayer && !VulkanUtils::checkValidationLayerSupport(VALIDATION_LAYERS))
        throw RendererError("Validation layers requested, but not available");

    // application info
    vk::ApplicationInfo appInfo = {};
    appInfo.setPApplicationName("Chronicle");
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setPEngineName("Chronicle");
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setApiVersion(VK_API_VERSION_1_0);

    // create instance info
    auto extensions = VulkanUtils::getRequiredExtensions();

    vk::InstanceCreateInfo createInfo = {};
    createInfo.setPApplicationInfo(&appInfo);
    createInfo.setPEnabledExtensionNames(extensions);

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (VulkanContext::enabledValidationLayer) {
        populateDebugMessengerCreateInfo(debugCreateInfo);

        createInfo.setPEnabledLayerNames(VALIDATION_LAYERS);
        createInfo.setPNext(&debugCreateInfo);
    }

    VulkanContext::instance = vk::createInstance(createInfo, nullptr);
}

void VulkanInstance::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
    CHRZONE_RENDERER;

    using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using enum vk::DebugUtilsMessageTypeFlagBitsEXT;

    createInfo.setMessageSeverity(eVerbose | eInfo | eWarning | eError);
    createInfo.setMessageType(eGeneral | eValidation | ePerformance);
    createInfo.setPfnUserCallback(debugCallback);
}

void VulkanInstance::setupDebugCallback()
{
    CHRZONE_RENDERER;

    if (!VulkanContext::enabledValidationLayer)
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
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create surface");

#ifdef GLFW_PLATFORM
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

    auto devices = VulkanContext::instance.enumeratePhysicalDevices();
    if (devices.size() == 0)
        throw RendererError("Failed to find GPUs with Vulkan support");

    for (const auto& device : devices) {
        if (VulkanUtils::isDeviceSuitable(device, DEVICE_EXTENSIONS)) {
            VulkanContext::physicalDevice = device;
            VulkanContext::msaaSamples = VulkanUtils::getMaxUsableSampleCount();
            break;
        }
    }

    if (!VulkanContext::physicalDevice)
        throw RendererError("Failed to find a suitable GPU");
}

void VulkanInstance::createLogicalDevice()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create logical device");

    VulkanQueueFamilyIndices indices = VulkanUtils::findQueueFamilies(VulkanContext::physicalDevice);

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

    if (VulkanContext::enabledValidationLayer)
        createInfo.setPEnabledLayerNames(VALIDATION_LAYERS);

    VulkanContext::device = VulkanContext::physicalDevice.createDevice(createInfo);

    VulkanContext::graphicsQueue = VulkanContext::device.getQueue(indices.graphicsFamily.value(), 0);
    VulkanContext::presentQueue = VulkanContext::device.getQueue(indices.presentFamily.value(), 0);
    VulkanContext::graphicsFamily = indices.graphicsFamily.value();
    VulkanContext::presentFamily = indices.presentFamily.value();
}

void VulkanInstance::createSwapChain()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create swapchain");

    auto swapChainSupport = VulkanUtils::querySwapChainSupport(VulkanContext::physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = VulkanUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = VulkanUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = VulkanUtils::chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    auto indices = VulkanUtils::findQueueFamilies(VulkanContext::physicalDevice);
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
    VulkanContext::swapChainImages = VulkanContext::device.getSwapchainImagesKHR(VulkanContext::swapChain);
    VulkanContext::swapChainImageFormat = surfaceFormat.format;
    VulkanContext::swapChainExtent = extent;

    VulkanContext::swapChainImageViews.reserve(VulkanContext::swapChainImages.size());
    for (const auto& swapChainImage : VulkanContext::swapChainImages) {
        auto swapChainImageView
            = VulkanUtils::createImageView(swapChainImage, surfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1);
        VulkanContext::swapChainImageViews.push_back(swapChainImageView);
    }
}

void VulkanInstance::createCommandPool()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create command pool");

    auto queueFamilyIndices = VulkanUtils::findQueueFamilies(VulkanContext::physicalDevice);
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

    // draw pass
    std::array<vk::AttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

    vk::RenderPassCreateInfo createRenderPassInfo = {};
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

    // draw pass
    std::array<vk::AttachmentDescription, 1> attachments = { colorAttachment };

    vk::RenderPassCreateInfo createRenderPassInfo = {};
    createRenderPassInfo.setAttachments(attachments);
    createRenderPassInfo.setSubpasses(subpass);
    createRenderPassInfo.setDependencies(dependency);

    VulkanContext::debugRenderPass = VulkanContext::device.createRenderPass(createRenderPassInfo);
}

void VulkanInstance::createFramebuffers()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create framebuffers");

    VulkanContext::framebuffers.reserve(VulkanContext::swapChainImageViews.size());

    for (const auto& swapChainImageView : VulkanContext::swapChainImageViews) {
        std::array<vk::ImageView, 3> attachments
            = { VulkanContext::colorImageView, VulkanContext::depthImageView, swapChainImageView };

        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.setRenderPass(VulkanContext::renderPass);
        framebufferInfo.setAttachments(attachments);
        framebufferInfo.setWidth(VulkanContext::swapChainExtent.width);
        framebufferInfo.setHeight(VulkanContext::swapChainExtent.height);
        framebufferInfo.setLayers(1);

        VulkanContext::framebuffers.push_back(VulkanContext::device.createFramebuffer(framebufferInfo));
    }
}

void VulkanInstance::createDebugFramebuffers()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create ImGui framebuffers");

    VulkanContext::debugFramebuffers.reserve(VulkanContext::swapChainImageViews.size());

    for (const auto& swapChainImageView : VulkanContext::swapChainImageViews) {
        std::array<vk::ImageView, 1> attachments = { swapChainImageView };

        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.setRenderPass(VulkanContext::debugRenderPass);
        framebufferInfo.setAttachments(attachments);
        framebufferInfo.setWidth(VulkanContext::swapChainExtent.width);
        framebufferInfo.setHeight(VulkanContext::swapChainExtent.height);
        framebufferInfo.setLayers(1);

        VulkanContext::debugFramebuffers.push_back(VulkanContext::device.createFramebuffer(framebufferInfo));
    }
}

void VulkanInstance::createSyncObjects()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create sync objects");

    VulkanContext::imageAvailableSemaphores.reserve(VulkanContext::maxFramesInFlight);
    VulkanContext::renderFinishedSemaphores.reserve(VulkanContext::maxFramesInFlight);
    VulkanContext::inFlightFences.reserve(VulkanContext::maxFramesInFlight);

    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        VulkanContext::imageAvailableSemaphores.push_back(VulkanContext::device.createSemaphore({}));
        VulkanContext::renderFinishedSemaphores.push_back(VulkanContext::device.createSemaphore({}));
        VulkanContext::inFlightFences.push_back(
            VulkanContext::device.createFence({ vk::FenceCreateFlagBits::eSignaled }));
    }
}

void VulkanInstance::createCommandBuffers()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create command buffers");

    VulkanContext::commandBuffers.reserve(VulkanContext::maxFramesInFlight);
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        auto commandBuffer = chronicle::VulkanCommandBuffer::create();
        VulkanContext::commandBuffers.push_back(commandBuffer);
    }
}

void VulkanInstance::createDescriptorSets()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create descriptor sets");

    VulkanContext::descriptorSets.reserve(VulkanContext::maxFramesInFlight);
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        auto descriptorSet = chronicle::DescriptorSet::create();
        descriptorSet->addUniform<UniformBufferObject>("ubo"_hs, chronicle::ShaderStage::Vertex);
        VulkanContext::descriptorSets.push_back(descriptorSet);
    }
}

void VulkanInstance::createDescriptorPool()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create descriptor pool");

    std::vector<vk::DescriptorPoolSize> sizes
        = { { vk::DescriptorType::eUniformBuffer, 1000 }, { vk::DescriptorType::eCombinedImageSampler, 1000 } };

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.setMaxSets(static_cast<uint32_t>(10000 * sizes.size()));
    poolInfo.setPoolSizes(sizes);

    VulkanContext::descriptorPool = VulkanContext::device.createDescriptorPool(poolInfo, nullptr);
}

} // namespace chronicle