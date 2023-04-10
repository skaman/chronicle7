#pragma once

#include "pch.h"

namespace chronicle {

class VulkanImageUtils {
public:
    static std::pair<vk::DeviceMemory, vk::Image> createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
        vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
    static vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);
    static vk::Sampler createTextureSampler();
};

} // namespace chronicle