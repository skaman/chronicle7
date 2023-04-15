// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanUtils.h"

#include "VulkanCommon.h"

#ifdef GLFW_PLATFORM
#include "Platform/GLFW/GLFWCommon.h"
#endif

namespace chronicle {

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

    vk::Extent3D imageExtent = {};
    imageExtent.setWidth(width);
    imageExtent.setHeight(height);
    imageExtent.setDepth(1);

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

    vkBindImageMemory(VulkanContext::device, image, imageMemory, 0);

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

    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(aspectFlags);
    subresourceRange.setBaseMipLevel(0);
    subresourceRange.setLevelCount(mipLevels);
    subresourceRange.setBaseArrayLayer(0);
    subresourceRange.setLayerCount(1);

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

    const auto properties = VulkanContext::physicalDevice.getProperties();

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

void VulkanUtils::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
    CHRZONE_RENDERER;

    assert(size > 0);
    assert(usage);
    assert(properties);

    CHRLOG_TRACE("Creating Vulkan buffer: size={}, usage={}, properties={}", size, vk::to_string(usage),
        vk::to_string(properties));

    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.setSize(size);
    bufferInfo.setUsage(usage);
    bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

    buffer = VulkanContext::device.createBuffer(bufferInfo);

    auto memRequirements = VulkanContext::device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

    bufferMemory = VulkanContext::device.allocateMemory(allocInfo);

    VulkanContext::device.bindBufferMemory(buffer, bufferMemory, 0);
}

void VulkanUtils::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    CHRZONE_RENDERER;

    assert(size > 0);
    assert(srcBuffer);
    assert(dstBuffer);

    CHRLOG_TRACE("Copying Vulkan buffer: size={}", size);

    auto commandBuffer = beginSingleTimeCommands();

    vk::BufferCopy copyRegion = {};
    copyRegion.setSize(size);

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

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

    auto commandBuffer = beginSingleTimeCommands();

    vk::ImageSubresourceLayers subresourceLayers = {};
    subresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresourceLayers.setMipLevel(0);
    subresourceLayers.setBaseArrayLayer(0);
    subresourceLayers.setLayerCount(1);

    vk::BufferImageCopy region = {};
    region.setBufferOffset(0);
    region.setBufferRowLength(0);
    region.setBufferImageHeight(0);
    region.setImageSubresource(subresourceLayers);
    region.setImageOffset({ 0, 0, 0 });
    region.setImageExtent({ width, height, 1 });

    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, region);

    endSingleTimeCommands(commandBuffer);
}

void VulkanUtils::transitionImageLayout(
    vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
{
    CHRZONE_RENDERER;

    assert(image);

    CHRLOG_TRACE("Transitioning Vulkan image layout: from={}, to={}, mip levels={}", vk::to_string(oldLayout),
        vk::to_string(newLayout), mipLevels);

    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresourceRange.setBaseMipLevel(0);
    subresourceRange.setLevelCount(mipLevels);
    subresourceRange.setBaseArrayLayer(0);
    subresourceRange.setLayerCount(1);

    vk::ImageMemoryBarrier barrier = {};
    barrier.setOldLayout(oldLayout);
    barrier.setNewLayout(newLayout);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setImage(image);
    barrier.setSubresourceRange(subresourceRange);

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

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

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

    auto commandBuffer = beginSingleTimeCommands();

    vk::ImageSubresourceRange subresource = {};
    subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresource.setBaseArrayLayer(0);
    subresource.setLayerCount(1);
    subresource.setLevelCount(1);

    vk::ImageMemoryBarrier barrier = {};
    barrier.setImage(image);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setSubresourceRange(subresource);

    uint32_t mipWidth = width;
    uint32_t mipHeight = height;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.setBaseMipLevel(i - 1);
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
        barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(), nullptr, nullptr, barrier);

        std::array<vk::Offset3D, 2> srcOffsets = { vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth, mipHeight, 1) };
        std::array<vk::Offset3D, 2> dstOffsets = { vk::Offset3D(0, 0, 0),
            vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1) };

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

        barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
        barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlags(), nullptr, nullptr, barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.setBaseMipLevel(mipLevels - 1);
    barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
    barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
    barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlags(), nullptr, nullptr, barrier);

    endSingleTimeCommands(commandBuffer);
}

uint32_t VulkanUtils::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    CHRZONE_RENDERER;

    auto memProperties = VulkanContext::physicalDevice.getMemoryProperties();

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

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandPool(VulkanContext::commandPool);
    allocInfo.setCommandBufferCount(1);

    auto commandBuffer = VulkanContext::device.allocateCommandBuffers(allocInfo);

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer[0].begin(beginInfo);

    return commandBuffer[0];
}

void VulkanUtils::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    CHRZONE_RENDERER;

    assert(commandBuffer);

    CHRLOG_TRACE("Ending Vulkan single time command");

    commandBuffer.end();

    vk::SubmitInfo submitInfo = {};
    submitInfo.setCommandBuffers(commandBuffer);

    VulkanContext::graphicsQueue.submit(submitInfo, nullptr);
    VulkanContext::graphicsQueue.waitIdle();

    VulkanContext::device.freeCommandBuffers(VulkanContext::commandPool, commandBuffer);
}

bool VulkanUtils::checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    CHRZONE_RENDERER;

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
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#else
    throw RendererError("Not implemented");
#endif

    if (VulkanContext::enabledValidationLayer)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool VulkanUtils::isDeviceSuitable(const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

    VulkanQueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, extensions);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    const auto supportedFeatures = physicalDevice.getFeatures();
    return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanUtils::checkDeviceExtensionSupport(
    const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

    std::set<std::string, std::less<>> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto& extension : physicalDevice.enumerateDeviceExtensionProperties())
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

VulkanQueueFamilyIndices VulkanUtils::findQueueFamilies(vk::PhysicalDevice physicalDevice)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

    VulkanQueueFamilyIndices indices;

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if (queueFamily.queueCount > 0 && physicalDevice.getSurfaceSupportKHR(i, VulkanContext::surface))
            indices.presentFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

VulkanSwapChainSupportDetails VulkanUtils::querySwapChainSupport(const vk::PhysicalDevice& physicalDevice)
{
    CHRZONE_RENDERER;

    assert(physicalDevice);

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

    using enum vk::Format;

    if (availableFormats.size() == 1 && availableFormats[0].format == eUndefined)
        return { eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    }

    return availableFormats[0];
}

vk::PresentModeKHR VulkanUtils::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    CHRZONE_RENDERER;

    assert(availablePresentModes.size() > 0);

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

vk::Extent2D VulkanUtils::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    CHRZONE_RENDERER;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width;
        int height;

#ifdef GLFW_PLATFORM
        glfwGetFramebufferSize(GLFWContext::window, &width, &height);
#else
        throw RendererError("Not implemented");
#endif

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
    using enum vk::Format;

    return findSupportedFormat({ eD32Sfloat, eD32SfloatS8Uint, eD24UnormS8Uint }, vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool VulkanUtils::hasStencilComponent(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

vk::SampleCountFlagBits VulkanUtils::getMaxUsableSampleCount()
{
    auto physicalDeviceProperties = VulkanContext::physicalDevice.getProperties();

    auto counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
        & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

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

} // namespace chronicle