// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanBuffer.h"

#include "Graphics/Common.h"
#include "VulkanDevice.h"

#include <magic_enum_flags.hpp>

namespace chronicle::graphics::internal::vulkan
{

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, const BufferDescriptor &bufferDescriptor)
    : Buffer(bufferDescriptor), _device(device)
{
    if (descriptor().size == 0)
    {
        throw BufferError("Can't create a buffer of 0 bytes");
    }

    _isHostVisible = magic_enum::enum_flags_test_any(descriptor().usage, BufferUsageFlags::eMapRead) ||
                     magic_enum::enum_flags_test_any(descriptor().usage, BufferUsageFlags::eMapWrite);

    auto memoryProperties = _isHostVisible
                                ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
                                : vk::MemoryPropertyFlagBits::eDeviceLocal;

    if (!_isHostVisible && descriptor().mappedAtCreation)
    {
        throw BufferError("Can't create a mapped buffer when it's no host accessible (see eMapRead/eMapWrite)");
    }

    // Prepare buffer usage flags.
    vk::BufferUsageFlags bufferUsage{};
    if (magic_enum::enum_flags_test_any(descriptor().usage, BufferUsageFlags::eCopySrc))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eTransferSrc;
    }
    if (magic_enum::enum_flags_test_any(descriptor().usage, BufferUsageFlags::eCopyDst))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eTransferDst;
    }
    if (magic_enum::enum_flags_test_any(descriptor().usage, BufferUsageFlags::eIndex))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
    }
    if (magic_enum::enum_flags_test_any(descriptor().usage, BufferUsageFlags::eVertex))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
    }
    if (magic_enum::enum_flags_test_any(descriptor().usage, BufferUsageFlags::eUniform))
    {
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
    }

    // Create the buffer.
    try
    {
        vk::BufferCreateInfo bufferInfo({}, descriptor().size, bufferUsage, vk::SharingMode::eExclusive);
        _vulkanBuffer = _device->vulkanLogicalDevice().createBuffer(bufferInfo);
    }
    catch (const vk::Error &error)
    {
        throw BufferError(fmt::format("Can't create the buffer: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eBuffer, (uint64_t)(VkBuffer)_vulkanBuffer, descriptor().name);
#endif

    /// Allocate memory.
    try
    {
        auto memRequirements = _device->vulkanLogicalDevice().getBufferMemoryRequirements(_vulkanBuffer);

        vk::MemoryAllocateInfo allocInfo(memRequirements.size,
                                         _device->findMemoryType(memRequirements.memoryTypeBits, memoryProperties));
        _vulkanMemory = _device->vulkanLogicalDevice().allocateMemory(allocInfo);
    }
    catch (const vk::Error &error)
    {
        _device->vulkanLogicalDevice().destroyBuffer(_vulkanBuffer);
        throw BufferError(fmt::format("Can't allocate memory for the buffer: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eDeviceMemory, (uint64_t)(VkDeviceMemory)_vulkanMemory,
                               descriptor().name);
#endif

    /// Bind memory and buffer together.
    try
    {
        _device->vulkanLogicalDevice().bindBufferMemory(_vulkanBuffer, _vulkanMemory, 0);
    }
    catch (const vk::Error &error)
    {
        _device->vulkanLogicalDevice().destroyBuffer(_vulkanBuffer);
        _device->vulkanLogicalDevice().freeMemory(_vulkanMemory);
        throw BufferError(fmt::format("Can't bind the buffer with his memory: {}", error.what()));
    }
}

VulkanBuffer::~VulkanBuffer()
{
    if (!_memoryMap.empty())
    {
        _device->vulkanLogicalDevice().unmapMemory(_vulkanMemory);
    }

    _device->vulkanLogicalDevice().destroyBuffer(_vulkanBuffer);
    _device->vulkanLogicalDevice().freeMemory(_vulkanMemory);
}

void VulkanBuffer::map()
{
    if (!_isHostVisible)
    {
        throw BufferError("Can't map a buffer when it's no host accessible (see eMapRead/eMapWrite)");
    }

    if (_isMapped)
    {
        return;
    }

    uint8_t *mappedMemory = nullptr;
    try
    {
        mappedMemory =
            static_cast<uint8_t *>(_device->vulkanLogicalDevice().mapMemory(_vulkanMemory, 0, descriptor().size));
    }
    catch (const vk::Error &error)
    {
        throw BufferError(fmt::format("Can't map the buffer: {}", error.what()));
    }

    _memoryMap = std::span<uint8_t>(mappedMemory, descriptor().size);
    _isMapped = true;
}

void VulkanBuffer::unmap()
{
    if (!_isMapped)
    {
        return;
    }

    _device->vulkanLogicalDevice().unmapMemory(_vulkanMemory);
    _memoryMap = std::span<uint8_t>{};
    _isMapped = false;
}

} // namespace chronicle::graphics::internal::vulkan