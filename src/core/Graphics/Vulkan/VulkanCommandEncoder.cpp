// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanCommandEncoder.h"

#include "VulkanBuffer.h"
#include "VulkanDevice.h"

namespace chronicle::graphics::internal::vulkan
{

VulkanCommandEncoder::VulkanCommandEncoder(std::shared_ptr<VulkanDevice> device,
                                           const CommandEncoderCreateInfo &commandEncoderCreateInfo)
    : _device(device), _name(commandEncoderCreateInfo.name)
{
    vk::CommandBufferAllocateInfo allocInfo(_device->vulkanCommandPool(), vk::CommandBufferLevel::ePrimary, 1);
    _commandBuffer = _device->vulkanLogicalDevice().allocateCommandBuffers(allocInfo)[0];

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eCommandBuffer, (uint64_t)(VkCommandBuffer)_commandBuffer, _name);
#endif
}

VulkanCommandEncoder::~VulkanCommandEncoder()
{
    _device->vulkanLogicalDevice().freeCommandBuffers(_device->vulkanCommandPool(), _commandBuffer);
}

void VulkanCommandEncoder::copyBufferToBuffer(const std::shared_ptr<Buffer> &source, uint64_t sourceOffset,
                                              const std::shared_ptr<Buffer> &destination, uint64_t destinationOffset,
                                              uint64_t size)
{
    vk::BufferCopy copyRegion(sourceOffset, destinationOffset, size);
    _commandBuffer.copyBuffer(static_cast<VulkanBuffer *>(source.get())->vulkanBuffer(),
                              static_cast<VulkanBuffer *>(destination.get())->vulkanBuffer(), copyRegion);
}

} // namespace chronicle::graphics::internal::vulkan