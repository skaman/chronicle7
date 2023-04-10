#include "VulkanVertexBuffer.h"

#include "VulkanBuffer.h"
#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanVertexBuffer)

VulkanVertexBuffer::VulkanVertexBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
    const vk::CommandPool& commandPool, const vk::Queue& queue)
    : _device(device)
    , _physicalDevice(physicalDevice)
    , _commandPool(commandPool)
    , _queue(queue)
{
}

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
    VulkanBuffer::create(_device, _physicalDevice, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    void* data = _device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, src, bufferSize);
    _device.unmapMemory(stagingBufferMemory);

    VulkanBuffer::create(_device, _physicalDevice, bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _buffer, _bufferMemory);

    VulkanBuffer::copy(_device, _commandPool, _queue, stagingBuffer, _buffer, bufferSize);

    _device.destroyBuffer(stagingBuffer);
    _device.freeMemory(stagingBufferMemory);
}

VertexBufferRef VulkanVertexBuffer::create(const Renderer* renderer)
{
    const auto vulkanInstance = static_cast<const VulkanInstance*>(renderer);
    return std::make_shared<ConcreteVulkanVertexBuffer>(vulkanInstance->device(), vulkanInstance->physicalDevice(),
        vulkanInstance->commandPool(), vulkanInstance->graphicsQueue());
}

void VulkanVertexBuffer::cleanup() const
{
    CHRZONE_VULKAN

    _device.destroyBuffer(_buffer);
    _device.freeMemory(_bufferMemory);
}

} // namespace chronicle