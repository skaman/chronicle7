// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanVertexBuffer.h"

#include "VulkanInstance.h"
#include "VulkanUtils.h"

namespace chronicle {

CHR_CONCRETE(VulkanVertexBuffer);

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy vertex buffer");

    // clean resources if needed
    if (_buffer)
        cleanup();
}

void VulkanVertexBuffer::set(void* src, size_t size)
{
    CHRZONE_RENDERER;

    assert(src != nullptr);
    assert(size > 0);

    CHRLOG_DEBUG("Set vertex buffer data: size={}", size);

    // clean resources if needed
    if (_buffer)
        cleanup();

    // create a buffer visible to the host
    vk::DeviceSize bufferSize = size;
    auto [stagingBufferMemory, stagingBuffer]
        = VulkanUtils::createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    // copy data to buffer
    void* data = VulkanContext::device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, src, bufferSize);
    VulkanContext::device.unmapMemory(stagingBufferMemory);

    // create a buffer visible only from the GPU
    auto [bufferMemory, buffer] = VulkanUtils::createBuffer(bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    _buffer = buffer;
    _bufferMemory = bufferMemory;

    // copy data from local visible buffer to GPU visible buffer
    VulkanUtils::copyBuffer(stagingBuffer, _buffer, bufferSize);

    // destroy local visible buffer
    VulkanContext::device.destroyBuffer(stagingBuffer);
    VulkanContext::device.freeMemory(stagingBufferMemory);
}

VertexBufferRef VulkanVertexBuffer::create()
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanVertexBuffer>();
}

void VulkanVertexBuffer::cleanup() const
{
    CHRZONE_RENDERER;

    // get garbage collector
    auto& garbageCollector = VulkanContext::framesData[VulkanContext::currentFrame].garbageCollector;

    // destroy buffer and free memory
    garbageCollector.emplace_back(_buffer);
    garbageCollector.emplace_back(_bufferMemory);
}

} // namespace chronicle