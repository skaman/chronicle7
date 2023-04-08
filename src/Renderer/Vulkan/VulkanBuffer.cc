#include "VulkanBuffer.h"

#include "Renderer/RendererError.h"

namespace chronicle {

void VulkanBuffer::create(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size,
    vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
    CHRZONE_VULKAN

    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.setSize(size);
    bufferInfo.setUsage(usage);
    bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

    buffer = device.createBuffer(bufferInfo);

    auto memRequirements = device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties));

    bufferMemory = device.allocateMemory(allocInfo);

    device.bindBufferMemory(buffer, bufferMemory, 0);
}

void VulkanBuffer::copy(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue,
    vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    CHRZONE_VULKAN

    auto commandBuffer = beginSingleTimeCommands(device, commandPool);

    const auto& copyRegion = vk::BufferCopy().setSize(size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
}

void VulkanBuffer::copyToImage(const vk::Device& device, const vk::CommandPool& commandPool,
    const vk::Queue& graphicsQueue, vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height)
{
    CHRZONE_VULKAN

    auto commandBuffer = beginSingleTimeCommands(device, commandPool);

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

    endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
}

void VulkanBuffer::transitionImageLayout(const vk::Device& device, const vk::CommandPool& commandPool,
    const vk::Queue& graphicsQueue, vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout)
{
    CHRZONE_VULKAN

    vk::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresourceRange.setBaseMipLevel(0);
    subresourceRange.setLevelCount(1);
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

    endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
}

uint32_t VulkanBuffer::findMemoryType(
    const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    CHRZONE_VULKAN

    auto memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw RendererError("Failed to find suitable memory type");
}

vk::CommandBuffer VulkanBuffer::beginSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool)
{
    CHRZONE_VULKAN

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandPool(commandPool);
    allocInfo.setCommandBufferCount(1);

    auto commandBuffer = device.allocateCommandBuffers(allocInfo)[0];
    const auto& beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void VulkanBuffer::endSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool,
    const vk::Queue& graphicsQueue, vk::CommandBuffer commandBuffer)
{
    CHRZONE_VULKAN

    commandBuffer.end();

    const auto& submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer);

    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
}

} // namespace chronicle