#pragma once

#include "pch.h"

#include "Renderer/IndexBufferI.h"

namespace chronicle {

class VulkanIndexBuffer : public IndexBufferI<VulkanIndexBuffer>, private NonCopyable<VulkanIndexBuffer> {
protected:
    explicit VulkanIndexBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
        const vk::CommandPool& commandPool, const vk::Queue& queue);

public:
    ~VulkanIndexBuffer();

    void set(void* src, size_t size);

    [[nodiscard]] inline const vk::Buffer& buffer() const { return _buffer; }

    static IndexBufferRef create(const Renderer* renderer);

private:
    vk::Device _device;
    vk::PhysicalDevice _physicalDevice;
    vk::CommandPool _commandPool;
    vk::Queue _queue;

    vk::Buffer _buffer;
    vk::DeviceMemory _bufferMemory;

    void cleanup() const;
};

} // namespace chronicle