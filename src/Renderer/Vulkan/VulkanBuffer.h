#pragma once

#include "pch.h"

namespace chronicle {

class VulkanBuffer {
public:
    static void create(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size,
        vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
        vk::DeviceMemory& bufferMemory);
    static void copy(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue,
        VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:
    [[nodiscard]] static uint32_t findMemoryType(
        const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};

} // namespace chronicle