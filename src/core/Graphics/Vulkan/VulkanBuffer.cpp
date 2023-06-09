// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanBuffer.h"

#include "Graphics/Common.h"
#include "VulkanDevice.h"

#include <magic_enum_flags.hpp>

namespace chronicle::graphics::internal::vulkan
{

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, const BufferCreateInfo &bufferCreateInfo)
    : _device(device), _name(bufferCreateInfo.name)
{
    assert(bufferCreateInfo.size > 0);
    assert(static_cast<int>(bufferCreateInfo.bufferUsage) != 0);

    auto isHostVisible = magic_enum::enum_flags_test_any(bufferCreateInfo.bufferUsage, BufferUsageFlags::eMapRead) ||
                         magic_enum::enum_flags_test_any(bufferCreateInfo.bufferUsage, BufferUsageFlags::eMapWrite);

    auto memoryProperties = isHostVisible
                                ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
                                : vk::MemoryPropertyFlagBits::eDeviceLocal;

    vk::BufferUsageFlags bufferUsage{};
    if (magic_enum::enum_flags_test_any(bufferCreateInfo.bufferUsage, BufferUsageFlags::eCopySrc))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eTransferSrc;
    }
    if (magic_enum::enum_flags_test_any(bufferCreateInfo.bufferUsage, BufferUsageFlags::eCopyDst))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eTransferDst;
    }
    if (magic_enum::enum_flags_test_any(bufferCreateInfo.bufferUsage, BufferUsageFlags::eIndex))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
    }
    if (magic_enum::enum_flags_test_any(bufferCreateInfo.bufferUsage, BufferUsageFlags::eVertex))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
    }
    if (magic_enum::enum_flags_test_any(bufferCreateInfo.bufferUsage, BufferUsageFlags::eUniform))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
    }

    // create buffer
    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.setSize(bufferCreateInfo.size);
    bufferInfo.setUsage(bufferUsage);
    bufferInfo.setSharingMode(vk::SharingMode::eExclusive);
    _buffer = _device->vulkanLogicalDevice().createBuffer(bufferInfo);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eBuffer, (uint64_t)(VkBuffer)_buffer, _name);
#endif

    // get memory requirements
    auto memRequirements = _device->vulkanLogicalDevice().getBufferMemoryRequirements(_buffer);

    // allocate memory
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(_device->findMemoryType(memRequirements.memoryTypeBits, memoryProperties));
    _memory = _device->vulkanLogicalDevice().allocateMemory(allocInfo);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eDeviceMemory, (uint64_t)(VkDeviceMemory)_memory, _name);
#endif

    // bind buffer memory
    _device->vulkanLogicalDevice().bindBufferMemory(_buffer, _memory, 0);

    if (isHostVisible)
    {
        auto *mappedMemory =
            static_cast<uint8_t *>(_device->vulkanLogicalDevice().mapMemory(_memory, 0, bufferCreateInfo.size));
        _memoryMap = std::span<uint8_t>(mappedMemory, bufferCreateInfo.size);
    }
}

VulkanBuffer::~VulkanBuffer()
{
    if (!_memoryMap.empty())
    {
        _device->vulkanLogicalDevice().unmapMemory(_memory);
    }

    _device->vulkanLogicalDevice().destroyBuffer(_buffer);
    _device->vulkanLogicalDevice().freeMemory(_memory);
}

} // namespace chronicle::graphics::internal::vulkan