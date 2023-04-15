// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

class VulkanUtils {
public:
    static std::pair<vk::DeviceMemory, vk::Image> createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
        vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

    static vk::ImageView createImageView(
        vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);

    static vk::Sampler createTextureSampler(uint32_t mipLevels);

    static void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
        vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);

    static void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    static void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height);

    static void transitionImageLayout(
        vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

    static void generateMipmaps(
        vk::Image image, vk::Format format, uint32_t width, uint32_t height, uint32_t mipLevels);

    [[nodiscard]] static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    static vk::CommandBuffer beginSingleTimeCommands();

    static void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
};

} // namespace chronicle