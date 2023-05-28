// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanVertexBuffer.h"

#include "VulkanGC.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"

namespace chronicle::internal::vulkan {

CHR_CONCRETE(VulkanVertexBuffer);

VulkanVertexBuffer::VulkanVertexBuffer(const uint8_t* src, size_t size, const std::string& name)
    : _name(name)
{
    CHRZONE_RENDERER;

    assert(src != nullptr);
    assert(size > 0);

    CHRLOG_TRACE("Set vertex buffer data: size={}", size);

    // create a buffer visible to the host
    vk::DeviceSize bufferSize = size;
    auto [stagingBufferMemory, stagingBuffer]
        = VulkanUtils::createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    assert(stagingBuffer);
    assert(stagingBufferMemory);

    // copy data to buffer
    void* dst = VulkanContext::device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(dst, src, bufferSize);
    VulkanContext::device.unmapMemory(stagingBufferMemory);

    // create a buffer visible only from the GPU
    auto [bufferMemory, buffer] = VulkanUtils::createBuffer(bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    assert(buffer);
    assert(bufferMemory);

    _buffer = buffer;
    _bufferMemory = bufferMemory;

    // copy data from local visible buffer to GPU visible buffer
    VulkanUtils::copyBuffer(stagingBuffer, _buffer, bufferSize);

    // destroy local visible buffer
    VulkanContext::device.destroyBuffer(stagingBuffer);
    VulkanContext::device.freeMemory(stagingBufferMemory);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::setDebugObjectName(_buffer, _name);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Destroy vertex buffer");

    // destroy buffer and free memory
    VulkanGC::add(_buffer);
    VulkanGC::add(_bufferMemory);
}

VertexBufferRef VulkanVertexBuffer::create(const std::vector<uint8_t>& data, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanVertexBuffer>(data.data(), data.size(), name);
}

VertexBufferRef VulkanVertexBuffer::create(const uint8_t* src, size_t size, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanVertexBuffer>(src, size, name);
}

} // namespace chronicle