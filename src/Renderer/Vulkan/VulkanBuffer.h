#pragma once

#include "pch.h"

namespace chronicle {

class VulkanBuffer {
public:
    static void create(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size,
        vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
        vk::DeviceMemory& bufferMemory);

    static void copy(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue,
        vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    static void copyToImage(const vk::Device& device, const vk::CommandPool& commandPool,
        const vk::Queue& graphicsQueue, vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height);

    static void transitionImageLayout(const vk::Device& device, const vk::CommandPool& commandPool,
        const vk::Queue& graphicsQueue, vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout, uint32_t mipLevels);

    [[nodiscard]] static uint32_t findMemoryType(
        const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

private:
    static vk::CommandBuffer beginSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool);
    static void endSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool,
        const vk::Queue& graphicsQueue, vk::CommandBuffer commandBuffer);
};

} // namespace chronicle