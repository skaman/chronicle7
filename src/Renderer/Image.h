#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanImage.h"
#endif

namespace chronicle {

class Renderer;

class Image {
public:
    Image(const Renderer* renderer, const ImageInfo& imageInfo);

#ifdef VULKAN_RENDERER
    Image(const vk::Device& device, const vk::Image& image, vk::Format format, int width, int height);
#endif

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline VulkanImage& native() { return _image; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanImage _image;
#endif
};

} // namespace chronicle