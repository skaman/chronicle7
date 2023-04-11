#include "VulkanIndexBuffer.h"

#include "VulkanInstance.h"
#include "VulkanUtils.h"

namespace chronicle {

CHR_CONCRETE(VulkanIndexBuffer)

VulkanIndexBuffer::~VulkanIndexBuffer()
{
    CHRZONE_RENDERER

    if (_buffer)
        cleanup();
}

void VulkanIndexBuffer::set(void* src, size_t size)
{
    CHRZONE_RENDERER

    vk::DeviceSize bufferSize = size;

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    VulkanUtils::createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    void* data = VulkanContext::device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, src, bufferSize);
    VulkanContext::device.unmapMemory(stagingBufferMemory);

    VulkanUtils::createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _buffer, _bufferMemory);

    VulkanUtils::copyBuffer(stagingBuffer, _buffer, bufferSize);

    VulkanContext::device.destroyBuffer(stagingBuffer);
    VulkanContext::device.freeMemory(stagingBufferMemory);
}

IndexBufferRef VulkanIndexBuffer::create() { return std::make_shared<ConcreteVulkanIndexBuffer>(); }

void VulkanIndexBuffer::cleanup() const
{
    CHRZONE_RENDERER

    VulkanContext::device.destroyBuffer(_buffer);
    VulkanContext::device.freeMemory(_bufferMemory);
}

} // namespace chronicle