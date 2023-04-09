#pragma once

#include "pch.h"

#include "Renderer/ImageI.h"

namespace chronicle {

class VulkanImage : public ImageI<VulkanImage>, private NonCopyable<VulkanImage> {
protected:
    explicit VulkanImage() = default;

public:
    ~VulkanImage();

    [[nodiscard]] uint32_t width() const { return _width; }
    [[nodiscard]] uint32_t height() const { return _height; }

    [[nodiscard]] const vk::Image& image() const { return _image; }
    [[nodiscard]] const vk::ImageView& imageView() const { return _imageView; }
    [[nodiscard]] const vk::Sampler& sampler() const { return _sampler; }

    void updateSwapchain(const vk::Image& image, vk::Format format, int width, int height);
    void updateDepthBuffer(uint32_t width, uint32_t height, vk::Format format);

    static ImageRef createTexture(const Renderer* renderer, const ImageInfo& imageInfo);
    static ImageRef createSwapchain(
        const vk::Device& device, const vk::Image& image, vk::Format format, int width, int height);
    static ImageRef createDepthBuffer(
        const vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, vk::Format format);

    entt::delegate<void(void)> updated {};

private:
    vk::Device _device;
    vk::PhysicalDevice _physicalDevice;
    vk::DeviceMemory _imageMemory;
    vk::Image _image;
    vk::ImageView _imageView;
    vk::Sampler _sampler;

    bool _swapchainImage = false;

    uint32_t _width;
    uint32_t _height;

    void cleanup();

    void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
        vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
    void createImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void createTextureSampler();
};

} // namespace chronicle