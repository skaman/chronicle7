#pragma once

#include "pch.h"

#include "Renderer/VertexBufferI.h"

namespace chronicle {

class VulkanVertexBuffer : public VertexBufferI<VulkanVertexBuffer>, private NonCopyable<VulkanVertexBuffer> {
protected:
    explicit VulkanVertexBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
        const vk::CommandPool& commandPool, const vk::Queue& queue);

public:
    ~VulkanVertexBuffer();

    void set(void* src, size_t size);

    [[nodiscard]] const vk::Buffer& buffer() const { return _buffer; }

    static VertexBufferRef create(const Renderer* renderer);

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