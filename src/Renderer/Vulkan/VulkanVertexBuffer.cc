#include "VulkanVertexBuffer.h"

#include "VulkanInstance.h"
#include "VulkanUtils.h"

namespace chronicle {

CHR_CONCRETE(VulkanVertexBuffer)

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    CHRZONE_VULKAN

    if (_buffer)
        cleanup();
}

void VulkanVertexBuffer::set(void* src, size_t size)
{
    CHRZONE_VULKAN

    if (_buffer)
        cleanup();

    vk::DeviceSize bufferSize = size;

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    VulkanUtils::createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    void* data = VulkanContext::device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, src, bufferSize);
    VulkanContext::device.unmapMemory(stagingBufferMemory);

    VulkanUtils::createBuffer(bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _buffer, _bufferMemory);

    VulkanUtils::copyBuffer(stagingBuffer, _buffer, bufferSize);

    VulkanContext::device.destroyBuffer(stagingBuffer);
    VulkanContext::device.freeMemory(stagingBufferMemory);
}

VertexBufferRef VulkanVertexBuffer::create() { return std::make_shared<ConcreteVulkanVertexBuffer>(); }

void VulkanVertexBuffer::cleanup() const
{
    CHRZONE_VULKAN

    VulkanContext::device.destroyBuffer(_buffer);
    VulkanContext::device.freeMemory(_bufferMemory);
}

} // namespace chronicle