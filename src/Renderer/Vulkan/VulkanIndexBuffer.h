#pragma once

#include "pch.h"

namespace chronicle {

class VulkanIndexBuffer {
public:
    explicit VulkanIndexBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
        const vk::CommandPool& commandPool, const vk::Queue& queue);
    ~VulkanIndexBuffer();

    void set(void* src, size_t size);

    // internal
    [[nodiscard]] inline const vk::Buffer& buffer() const { return _buffer; }

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