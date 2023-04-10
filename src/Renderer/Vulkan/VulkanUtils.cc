#include "VulkanUtils.h"

#include "VulkanCommon.h"

namespace chronicle {

std::pair<vk::DeviceMemory, vk::Image> VulkanUtils::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
    vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    CHRZONE_VULKAN

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
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);

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
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

    auto commandBuffer = beginSingleTimeCommands();

    const auto& copyRegion = vk::BufferCopy().setSize(size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void VulkanUtils::copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height)
{
    CHRZONE_VULKAN

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

void VulkanUtils::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
{
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

    commandBuffer.end();

    vk::SubmitInfo submitInfo = {};
    submitInfo.setCommandBuffers(commandBuffer);

    VulkanContext::graphicsQueue.submit(submitInfo, nullptr);
    VulkanContext::graphicsQueue.waitIdle();

    VulkanContext::device.freeCommandBuffers(VulkanContext::commandPool, commandBuffer);
}

} // namespace chronicle