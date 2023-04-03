#pragma once

#include "pch.h"

namespace chronicle {

class VulkanImage {
public:
    explicit VulkanImage(const vk::Device& device, const vk::Image& image, const vk::ImageView& imageView,
        int imageViewWidth, int imageViewHeight);
    ~VulkanImage();

    [[nodiscard]] inline uint32_t width() const { return _width; }
    [[nodiscard]] inline uint32_t height() const { return _height; }

    // internal
    [[nodiscard]] inline const vk::Image& image() const { return _image; }
    [[nodiscard]] inline const vk::ImageView& imageView() const { return _imageView; }

    void updateImage(const vk::Image& image, const vk::ImageView& imageView, int imageViewWidth, int imageViewHeight);

    entt::delegate<void(void)> updated {};

private:
    vk::Device _device;
    vk::Image _image;
    vk::ImageView _imageView;

    uint32_t _width;
    uint32_t _height;
};

} // namespace chronicle