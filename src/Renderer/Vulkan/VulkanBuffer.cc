#include "VulkanBuffer.h"

#include "Renderer/RendererError.h"

namespace chronicle {

void VulkanBuffer::create(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size,
    vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
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
    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandPool(commandPool);
    allocInfo.setCommandBufferCount(1);

    auto commandBuffer = device.allocateCommandBuffers(allocInfo)[0];
    const auto& beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    const auto& copyRegion = vk::BufferCopy().setSize(size);

    commandBuffer.begin(beginInfo);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    commandBuffer.end();

    const auto& submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer);

    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
}

uint32_t VulkanBuffer::findMemoryType(
    const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    auto memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw RendererError("Failed to find suitable memory type");
}

} // namespace chronicle