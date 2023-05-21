// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanInstance.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanExtensions.h"
#include "VulkanFrameBuffer.h"
#include "VulkanGC.h"
#include "VulkanInstance.h"
#include "VulkanRenderPass.h"
#include "VulkanUtils.h"

#ifdef GLFW_PLATFORM
#include "Platform/GLFW/GLFWCommon.h"
#endif

using namespace entt::literals;

namespace chronicle {

const std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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

    CHRLOG_TRACE("Vulkan instance init");

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
    createFramebuffers();
    createSyncObjects();
    createCommandBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void VulkanInstance::deinit()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Vulkan instance deinit");

    // reset descriptor sets references
    // this must be happen before to clean the garbage collector
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        VulkanContext::framesData[i].descriptorSet.reset();
    }

    // clean data from frames garbage collectors
    VulkanGC::cleanupAll();

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
    VulkanContext::renderPass.reset();

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

    CHRLOG_TRACE("Recreate swapchain");

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
}

void VulkanInstance::cleanupSwapChain()
{
    CHRZONE_RENDERER;

    // clean framebuffers
    for (auto& imageData : VulkanContext::imagesData) {
        imageData.framebuffer.reset();
    }

    // clean multisampling color image
    VulkanContext::colorTexture.reset();

    // clean depth image
    VulkanContext::depthTexture.reset();

    // clean image data
    VulkanContext::imagesData.clear();

    // destroy swapchain
    VulkanContext::device.destroySwapchainKHR(VulkanContext::swapChain);
}

void VulkanInstance::createInstance()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Create instance");

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
    // populate the data structure
    createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    createInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
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

    CHRLOG_TRACE("Create surface");

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
            // TODO: restore
            // VulkanContext::msaaSamples = VulkanUtils::getMaxUsableSampleCount();
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

    CHRLOG_TRACE("Create logical device");

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

    CHRLOG_TRACE("Create swapchain");

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
    VulkanContext::colorTexture
        = Texture::createColor({ .width = extent.width,
                                   .height = extent.height,
                                   .format = VulkanEnums::formatFromVulkan(surfaceFormat.format),
                                   .msaa = VulkanEnums::msaaFromVulkan(VulkanContext::msaaSamples) },
            "Main color texture");

    // create depth buffer
    VulkanContext::depthImageFormat = VulkanUtils::findDepthFormat();
    VulkanContext::depthTexture
        = Texture::createDepth({ .width = extent.width,
                                   .height = extent.height,
                                   .format = VulkanEnums::formatFromVulkan(VulkanContext::depthImageFormat),
                                   .msaa = VulkanEnums::msaaFromVulkan(VulkanContext::msaaSamples) },
            "Main depth texture");

    // create swap chain images
    auto swapChainImages = VulkanContext::device.getSwapchainImagesKHR(VulkanContext::swapChain);
    VulkanContext::swapChainImageFormat = surfaceFormat.format;
    VulkanContext::swapChainExtent = extent;

    // prepare images data structures
    VulkanContext::imagesData.resize(swapChainImages.size());
    for (auto i = 0; i < swapChainImages.size(); i++) {
        VulkanContext::imagesData[i].swapChainTexture = VulkanTexture::createSwapchain(
            swapChainImages[i], surfaceFormat.format, extent.width, extent.height, "Swapchain texture");
    }
}

void VulkanInstance::createCommandPool()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Create command pool");

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

    CHRLOG_TRACE("Create render pass");

    // color attachment
    RenderPassAttachment colorAttachment
        = { .format = VulkanEnums::formatFromVulkan(VulkanContext::swapChainImageFormat),
              .msaa = VulkanEnums::msaaFromVulkan(VulkanContext::msaaSamples),
              .loadOp = AttachmentLoadOp::clear,
              .storeOp = AttachmentStoreOp::store,
              .stencilLoadOp = AttachmentLoadOp::dontCare,
              .stencilStoreOp = AttachmentStoreOp::dontCare,
              .initialLayout = ImageLayout::undefined,
              .finalLayout = ImageLayout::colorAttachment };

    // depth attachment
    RenderPassAttachment depthAttachment = { .format = VulkanEnums::formatFromVulkan(VulkanContext::depthImageFormat),
        .msaa = VulkanEnums::msaaFromVulkan(VulkanContext::msaaSamples),
        .loadOp = AttachmentLoadOp::clear,
        .storeOp = AttachmentStoreOp::dontCare,
        .stencilLoadOp = AttachmentLoadOp::dontCare,
        .stencilStoreOp = AttachmentStoreOp::dontCare,
        .initialLayout = ImageLayout::undefined,
        .finalLayout = ImageLayout::depthStencilAttachment };

    // resolve attachment
    RenderPassAttachment resolveAttachment
        = { .format = VulkanEnums::formatFromVulkan(VulkanContext::swapChainImageFormat),
              .msaa = MSAA::sampleCount1,
              .loadOp = AttachmentLoadOp::dontCare,
              .storeOp = AttachmentStoreOp::store,
              .stencilLoadOp = AttachmentLoadOp::dontCare,
              .stencilStoreOp = AttachmentStoreOp::dontCare,
              .initialLayout = ImageLayout::undefined,
              .finalLayout = ImageLayout::presentSrc };

    // create the renderpass
    RenderPassInfo renderPassInfo = { .colorAttachment = colorAttachment,
        .depthStencilAttachment = depthAttachment,
        .resolveAttachment = resolveAttachment };
    VulkanContext::renderPass = RenderPass::create(renderPassInfo, "Main render pass");
}

void VulkanInstance::createFramebuffers()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Create framebuffers");

    // create the framebuffers
    for (auto& imageData : VulkanContext::imagesData) {
        FrameBufferInfo frameBufferInfo = {};
        frameBufferInfo.attachments = { VulkanContext::colorTexture->textureId(),
            VulkanContext::depthTexture->textureId(), imageData.swapChainTexture->textureId() };
        frameBufferInfo.renderPass = VulkanContext::renderPass->renderPassId();
        frameBufferInfo.width = VulkanContext::swapChainExtent.width;
        frameBufferInfo.height = VulkanContext::swapChainExtent.height;
        imageData.framebuffer = FrameBuffer::create(frameBufferInfo, "Main framebuffer");
    }
}

void VulkanInstance::createSyncObjects()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Create sync objects");

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

    CHRLOG_TRACE("Create command buffers");

    // create the command buffers
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        VulkanContext::framesData[i].commandBuffer
            = VulkanCommandBuffer::create(fmt::format("Main command buffer (frame {})", i));
    }
}

void VulkanInstance::createDescriptorSets()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Create descriptor sets");

    // create the descriptor sets
    for (auto i = 0; i < VulkanContext::maxFramesInFlight; i++) {
        auto descriptorSet = DescriptorSet::create(fmt::format("Main descriptor set (frame {})", i));
        descriptorSet->addUniform<UniformBufferObject>("ubo"_hs, ShaderStage::vertex);
        VulkanContext::framesData[i].descriptorSet = descriptorSet;
    }
}

void VulkanInstance::createDescriptorPool()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Create descriptor pool");

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