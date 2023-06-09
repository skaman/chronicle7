// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanUtils.h"

#include "VulkanCommon.h"
#include "VulkanExtensions.h"

#ifdef GLFW_PLATFORM
#include "Platform/GLFW/GLFWCommon.h"
#endif

namespace chronicle::internal::vulkan {

std::pair<vk::DeviceMemory, vk::Image> VulkanUtils::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
    vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags properties)
{
    CHRZONE_RENDERER;

    assert(width > 0);
    assert(height > 0);
    assert(format != vk::Format::eUndefined);

    CHRLOG_TRACE("Creating Vulkan image: size={}x{}, mip levels={}, format={}, tiling={}, usage={}, properties={}",
        width, height, mipLevels, vk::to_string(format), vk::to_string(tiling), vk::to_string(usage),
        vk::to_string(properties));

    // image extent
    vk::Extent3D imageExtent = {};
    imageExtent.setWidth(width);
    imageExtent.setHeight(height);
    imageExtent.setDepth(1);

    // create image
    vk::ImageCreateInfo imageCreateInfo {};
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setExtent(imageExtent);
    imageCreateInfo.setMipLevels(mipLevels);
    imageCreateInfo.setArrayLayers(1);
    imageCreateInfo.setFormat(format);
    imageCreateInfo.setTiling(tiling);
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setUsage(usage);
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
    imageCreateInfo.setSamples(numSamples);
    auto image = VulkanContext::device.createImage(imageCreateInfo);

    // allocate memory
    const auto memRequirements = VulkanContext::device.getImageMemoryRequirements(image);
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));
    auto imageMemory = VulkanContext::device.allocateMemory(allocInfo, nullptr);

    // bind image memory
    vkBindImageMemory(VulkanContext::device, image, imageMemory, 0);

    // return data
    return { imageMemory, image };
}

vk::ImageView VulkanUtils::createImageView(
    vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    CHRZONE_RENDERER;

    assert(image);
    assert(format != vk::Format::eUndefined);

    CHRLOG_TRACE("Creating Vulkan image view: format={}, aspectFlags={}, mipLevels={}", vk::to_string(format),
        vk::to_string(aspectFlags), mipLevels);

    // image subresource range
    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(aspectFlags);
    subresourceRange.setBaseMipLevel(0);
    subresourceRange.setLevelCount(mipLevels);
    subresourceRange.setBaseArrayLayer(0);
    subresourceRange.setLayerCount(1);

    // create image view
    vk::ImageViewCreateInfo viewInfo = {};
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.setSubresourceRange(subresourceRange);
    return VulkanContext::device.createImageView(viewInfo);
}

vk::Sampler VulkanUtils::createTextureSampler(uint32_t mipLevels)
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Creating Vulkan texture sampler: mipLevels={}", mipLevels);

    // get device properties
    const auto properties = VulkanContext::physicalDevice.getProperties();

    // create sampler
    vk::SamplerCreateInfo samplerInfo = {};
    samplerInfo.setMagFilter(vk::Filter::eLinear);
    samplerInfo.setMinFilter(vk::Filter::eLinear);
    samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAnisotropyEnable(true);
    samplerInfo.setMaxAnisotropy(properties.limits.maxSamplerAnisotropy);
    samplerInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
    samplerInfo.setUnnormalizedCoordinates(false);
    samplerInfo.setCompareEnable(false);
    samplerInfo.setCompareOp(vk::CompareOp::eAlways);
    samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    samplerInfo.setMipLodBias(0.0f);
    samplerInfo.setMinLod(0.0f);
    samplerInfo.setMaxLod(static_cast<float>(mipLevels));
    return VulkanContext::device.createSampler(samplerInfo);
}

std::pair<vk::DeviceMemory, vk::Buffer> VulkanUtils::createBuffer(
    vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    CHRZONE_RENDERER;

    assert(size > 0);
    assert(usage);
    assert(properties);

    CHRLOG_TRACE("Creating Vulkan buffer: size={}, usage={}, properties={}", size, vk::to_string(usage),
        vk::to_string(properties));

    // create buffer
    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.setSize(size);
    bufferInfo.setUsage(usage);
    bufferInfo.setSharingMode(vk::SharingMode::eExclusive);
    auto buffer = VulkanContext::device.createBuffer(bufferInfo);

    // get memory requirements
    auto memRequirements = VulkanContext::device.getBufferMemoryRequirements(buffer);

    // allocate memory
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));
    auto bufferMemory = VulkanContext::device.allocateMemory(allocInfo);

    // bind buffer memory
    VulkanContext::device.bindBufferMemory(buffer, bufferMemory, 0);

    // return data
    return std::pair<vk::DeviceMemory, vk::Buffer>(bufferMemory, buffer);
}

void VulkanUtils::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    CHRZONE_RENDERER;

    assert(size > 0);
    assert(srcBuffer);
    assert(dstBuffer);

    CHRLOG_TRACE("Copying Vulkan buffer: size={}", size);

    // begin single time command buffer
    auto commandBuffer = beginSingleTimeCommands();

    // copy buffer
    vk::BufferCopy copyRegion = {};
    copyRegion.setSize(size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    // end single time command buffer
    endSingleTimeCommands(commandBuffer);
}

void VulkanUtils::copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height)
{
    CHRZONE_RENDERER;

    assert(width > 0);
    assert(height > 0);
    assert(srcBuffer);
    assert(dstImage);

    CHRLOG_TRACE("Copying Vulkan buffer to image: size={}x{}", width, height);

    // begin single time command buffer
    auto commandBuffer = beginSingleTimeCommands();

    // image subresource layers
    vk::ImageSubresourceLayers subresourceLayers = {};
    subresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresourceLayers.setMipLevel(0);
    subresourceLayers.setBaseArrayLayer(0);
    subresourceLayers.setLayerCount(1);

    // copy buffer to image
    vk::BufferImageCopy region = {};
    region.setBufferOffset(0);
    region.setBufferRowLength(0);
    region.setBufferImageHeight(0);
    region.setImageSubresource(subresourceLayers);
    region.setImageOffset({ 0, 0, 0 });
    region.setImageExtent({ width, height, 1 });
    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, region);

    // end single time command buffer
    endSingleTimeCommands(commandBuffer);
}

void VulkanUtils::transitionImageLayout(
    vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
{
    CHRZONE_RENDERER;

    assert(image);

    CHRLOG_TRACE("Transitioning Vulkan image layout: from={}, to={}, mip levels={}", vk::to_string(oldLayout),
        vk::to_string(newLayout), mipLevels);

    // begin single time command buffer
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    // image subresource range
    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresourceRange.setBaseMipLevel(0);
    subresourceRange.setLevelCount(mipLevels);
    subresourceRange.setBaseArrayLayer(0);
    subresourceRange.setLayerCount(1);

    // image memory barrier
    vk::ImageMemoryBarrier barrier = {};
    barrier.setOldLayout(oldLayout);
    barrier.setNewLayout(newLayout);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setImage(image);
    barrier.setSubresourceRange(subresourceRange);

    // detect source and destination data
    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlags());
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
        && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw RendererError("Unsupported layout transition");
    }

    // create pipeline barrier
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

    // end single time command buffer
    endSingleTimeCommands(commandBuffer);
}

void VulkanUtils::generateMipmaps(
    vk::Image image, vk::Format format, uint32_t width, uint32_t height, uint32_t mipLevels)
{
    CHRZONE_RENDERER;

    assert(image);

    CHRLOG_TRACE("Generating Vulkan mipmaps: size={}x{}, format={}, mip levels={}", width, height,
        vk::to_string(format), mipLevels);

    // Check if image format supports linear blitting
    if (auto formatProperties = VulkanContext::physicalDevice.getFormatProperties(format);
        !(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw RendererError("Texture image format does not support linear blitting");
    }

    // begin single time command buffer
    auto commandBuffer = beginSingleTimeCommands();

    // image subresource range
    vk::ImageSubresourceRange subresource = {};
    subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresource.setBaseArrayLayer(0);
    subresource.setLayerCount(1);
    subresource.setLevelCount(1);

    // image memory barrier
    vk::ImageMemoryBarrier barrier = {};
    barrier.setImage(image);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setSubresourceRange(subresource);

    // create every miplevel
    uint32_t mipWidth = width;
    uint32_t mipHeight = height;
    for (uint32_t i = 1; i < mipLevels; i++) {
        // pipeline barrier
        barrier.subresourceRange.setBaseMipLevel(i - 1);
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
        barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(), nullptr, nullptr, barrier);

        // set source and destination offsets
        std::array<vk::Offset3D, 2> srcOffsets = { vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth, mipHeight, 1) };
        std::array<vk::Offset3D, 2> dstOffsets = { vk::Offset3D(0, 0, 0),
            vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1) };

        // blit image
        vk::ImageBlit blit = {};
        blit.setSrcOffsets(srcOffsets);
        blit.srcSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
        blit.srcSubresource.setMipLevel(i - 1);
        blit.srcSubresource.setBaseArrayLayer(0);
        blit.srcSubresource.setLayerCount(1);
        blit.setDstOffsets(dstOffsets);
        blit.dstSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
        blit.dstSubresource.setMipLevel(i);
        blit.dstSubresource.setBaseArrayLayer(0);
        blit.dstSubresource.setLayerCount(1);
        commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image,
            vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

        // pipeline barrier
        barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
        barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlags(), nullptr, nullptr, barrier);

        // calculate next mipmap size
        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    // pipeline barrier
    barrier.subresourceRange.setBaseMipLevel(mipLevels - 1);
    barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
    barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
    barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlags(), nullptr, nullptr, barrier);

    // end single time command buffer
    endSingleTimeCommands(commandBuffer);
}

uint32_t VulkanUtils::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    CHRZONE_RENDERER;

    // get memory properties
    auto memProperties = VulkanContext::physicalDevice.getMemoryProperties();

    // get the first memory location with compatible flags
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw RendererError("Failed to find suitable memory type");
}

vk::CommandBuffer VulkanUtils::beginSingleTimeCommands()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Beginning Vulkan single time command");

    // allocate command buffer
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandPool(VulkanContext::commandPool);
    allocInfo.setCommandBufferCount(1);
    auto commandBuffer = VulkanContext::device.allocateCommandBuffers(allocInfo);

    // begin command buffer
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer[0].begin(beginInfo);

    // return command buffer
    return commandBuffer[0];
}

void VulkanUtils::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    CHRZONE_RENDERER;

    assert(commandBuffer);

    CHRLOG_TRACE("Ending Vulkan single time command");

    // end command buffer
    commandBuffer.end();

    // submit command buffer
    vk::SubmitInfo submitInfo = {};
    submitInfo.setCommandBuffers(commandBuffer);
    VulkanContext::graphicsQueue.submit(submitInfo, nullptr);

    // wait for queue idle
    VulkanContext::graphicsQueue.waitIdle();

    // free command buffer
    VulkanContext::device.freeCommandBuffers(VulkanContext::commandPool, commandBuffer);
}

bool VulkanUtils::checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    CHRZONE_RENDERER;

    // check validation layer support
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : validationLayers) {
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

std::vector<const char*> VulkanUtils::getRequiredExtensions()
{
    CHRZONE_RENDERER;

#ifdef GLFW_PLATFORM
    // get required extension from GLFW api
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#else
    throw RendererError("Not implemented");
#endif

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#else
    // if validation layer is enabled, add the debug utils extension
    if (VulkanContext::enabledValidationLayer)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // VULKAN_ENABLE_DEBUG_MARKER

    // return all the extensions
    return extensions;
}

bool VulkanUtils::isDeviceSuitable(const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

    // find queue families
    VulkanQueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    // check if the extensions are supported
    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, extensions);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // get supported features
    const auto supportedFeatures = physicalDevice.getFeatures();

    // check and return result
    return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy
        && supportedFeatures.fillModeNonSolid;
}

bool VulkanUtils::checkDeviceExtensionSupport(
    const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

    // copy the required extensions into a set
    std::set<std::string, std::less<>> requiredExtensions(extensions.begin(), extensions.end());

    //// get data from physical device
    // auto deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    // auto properties = physicalDevice.getProperties();
    //
    //// log all available extensions
    // CHRLOG_TRACE("Available device extensions on {}:", properties.deviceName);
    // for (const auto& extension : deviceExtensions) {
    //     CHRLOG_TRACE("- {}", extension.extensionName);
    // }

    // enumerate all the device extensions and remove from the set
    for (const auto& extension : physicalDevice.enumerateDeviceExtensionProperties())
        requiredExtensions.erase(extension.extensionName);

    // if there's still an extension into the set, the device doesn't support all the extensions.
    return requiredExtensions.empty();
}

VulkanQueueFamilyIndices VulkanUtils::findQueueFamilies(vk::PhysicalDevice physicalDevice)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

    // get family properties
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    // populate the queue family indices
    int i = 0;
    VulkanQueueFamilyIndices indices;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if (queueFamily.queueCount > 0 && physicalDevice.getSurfaceSupportKHR(i, VulkanContext::surface))
            indices.presentFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    // return data
    return indices;
}

VulkanSwapChainSupportDetails VulkanUtils::querySwapChainSupport(const vk::PhysicalDevice& physicalDevice)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

    // populate and return the swapchain support details
    VulkanSwapChainSupportDetails details;
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(VulkanContext::surface);
    details.formats = physicalDevice.getSurfaceFormatsKHR(VulkanContext::surface);
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(VulkanContext::surface);
    return details;
}

vk::SurfaceFormatKHR VulkanUtils::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    CHRZONE_RENDERER;

    assert(availableFormats.size() > 0);

    // if no available format is defined, just return a default
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
        return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

    // check and return the first available format that match
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm
            && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    }

    // return the first format if no one match
    return availableFormats[0];
}

vk::PresentModeKHR VulkanUtils::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    CHRZONE_RENDERER;

    assert(availablePresentModes.size() > 0);

    // check the best presentation mode
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
            return availablePresentMode;
        else if (availablePresentMode == vk::PresentModeKHR::eImmediate)
            bestMode = availablePresentMode;
    }
    return bestMode;
}

vk::Extent2D VulkanUtils::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    CHRZONE_RENDERER;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width;
        int height;

#ifdef GLFW_PLATFORM
        // get the framebuffer size
        glfwGetFramebufferSize(GLFWContext::window, &width, &height);
#else
        throw RendererError("Not implemented");
#endif

        // calculate and return the extent
        vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        actualExtent.width = std::max(
            capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

vk::Format VulkanUtils::findSupportedFormat(
    const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    assert(candidates.size() > 0);
    assert(features);

    // find and return the best supported format
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

vk::Format VulkanUtils::findDepthFormat()
{
    return findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool VulkanUtils::hasStencilComponent(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

vk::SampleCountFlagBits VulkanUtils::getMaxUsableSampleCount()
{
    // get device properties
    auto physicalDeviceProperties = VulkanContext::physicalDevice.getProperties();

    // get the supported samples flags
    auto counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
        & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    // return the highest one
    if (counts & vk::SampleCountFlagBits::e64) {
        return vk::SampleCountFlagBits::e64;
    }
    if (counts & vk::SampleCountFlagBits::e32) {
        return vk::SampleCountFlagBits::e32;
    }
    if (counts & vk::SampleCountFlagBits::e16) {
        return vk::SampleCountFlagBits::e16;
    }
    if (counts & vk::SampleCountFlagBits::e8) {
        return vk::SampleCountFlagBits::e8;
    }
    if (counts & vk::SampleCountFlagBits::e4) {
        return vk::SampleCountFlagBits::e4;
    }
    if (counts & vk::SampleCountFlagBits::e2) {
        return vk::SampleCountFlagBits::e2;
    }
    return vk::SampleCountFlagBits::e1;
}

void VulkanUtils::setDebugObjectName(vk::ObjectType objectType, uint64_t handle, const std::string& name)
{
    if (name.empty())
        return;

    VkDebugUtilsObjectNameInfoEXT objectNameInfo = {};
    objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    objectNameInfo.objectType = (VkObjectType)objectType;
    objectNameInfo.objectHandle = handle;
    objectNameInfo.pObjectName = name.c_str();
    objectNameInfo.pNext = nullptr;

    setDebugUtilsObjectNameEXT(VulkanContext::instance, VulkanContext::device, &objectNameInfo);
}

void VulkanUtils::setDebugObjectName(vk::Buffer buffer, const std::string& name)
{
    setDebugObjectName(vk::ObjectType::eBuffer, (uint64_t)(VkBuffer)buffer, name);
}

void VulkanUtils::setDebugObjectName(vk::CommandBuffer commandBuffer, const std::string& name)
{
    setDebugObjectName(vk::ObjectType::eCommandBuffer, (uint64_t)(VkCommandBuffer)commandBuffer, name);
}

void VulkanUtils::setDebugObjectName(vk::DescriptorSet descriptorSet, const std::string& name)
{
    setDebugObjectName(vk::ObjectType::eDescriptorSet, (uint64_t)(VkDescriptorSet)descriptorSet, name);
}

void VulkanUtils::setDebugObjectName(vk::Pipeline pipeline, const std::string& name)
{
    setDebugObjectName(vk::ObjectType::ePipeline, (uint64_t)(VkPipeline)pipeline, name);
}

void VulkanUtils::setDebugObjectName(vk::Framebuffer framebuffer, const std::string& name)
{
    setDebugObjectName(vk::ObjectType::eFramebuffer, (uint64_t)(VkFramebuffer)framebuffer, name);
}

void VulkanUtils::setDebugObjectName(vk::RenderPass renderPass, const std::string& name)
{
    setDebugObjectName(vk::ObjectType::eRenderPass, (uint64_t)(VkRenderPass)renderPass, name);
}

void VulkanUtils::beginDebugLabel(vk::CommandBuffer commandBuffer, const std::string& name, glm::vec4 color)
{
    if (name.empty())
        return;

    VkDebugUtilsLabelEXT debugLabelInfo = {};
    debugLabelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    debugLabelInfo.pLabelName = name.c_str();
    debugLabelInfo.color[0] = color.r;
    debugLabelInfo.color[1] = color.g;
    debugLabelInfo.color[2] = color.b;
    debugLabelInfo.color[3] = color.a;
    debugLabelInfo.pNext = nullptr;

    cmdBeginDebugUtilsLabelEXT(VulkanContext::instance, commandBuffer, &debugLabelInfo);
}

void VulkanUtils::endDebugLabel(vk::CommandBuffer commandBuffer)
{

    cmdEndDebugUtilsLabelEXT(VulkanContext::instance, commandBuffer);
}

void VulkanUtils::insertDebugLabel(vk::CommandBuffer commandBuffer, const std::string& name, glm::vec4 color)
{
    if (name.empty())
        return;

    VkDebugUtilsLabelEXT debugLabelInfo = {};
    debugLabelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    debugLabelInfo.pLabelName = name.c_str();
    debugLabelInfo.color[0] = color.r;
    debugLabelInfo.color[1] = color.g;
    debugLabelInfo.color[2] = color.b;
    debugLabelInfo.color[3] = color.a;
    debugLabelInfo.pNext = nullptr;

    cmdInsertDebugUtilsLabelEXT(VulkanContext::instance, commandBuffer, &debugLabelInfo);
}

} // namespace chronicle