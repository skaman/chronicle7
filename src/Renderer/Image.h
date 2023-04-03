#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanImage.h"
#endif

namespace chronicle {

class Image {
public:
#ifdef VULKAN_RENDERER
    Image(const vk::Device& device, const vk::Image& image, const vk::ImageView& imageView, int imageViewWidth,
        int imageViewHeight);
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