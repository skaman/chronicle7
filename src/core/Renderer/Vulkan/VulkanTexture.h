// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/TextureI.h"

namespace chronicle {

class VulkanTexture : public TextureI<VulkanTexture>, private NonCopyable<VulkanTexture> {
protected:
    explicit VulkanTexture() = default;

public:
    ~VulkanTexture();

    void set(void* src, size_t size, uint32_t width, uint32_t height);

    [[nodiscard]] uint32_t width() const { return _width; }
    [[nodiscard]] uint32_t height() const { return _height; }

    [[nodiscard]] const vk::Image& image() const { return _image; }
    [[nodiscard]] const vk::ImageView& imageView() const { return _imageView; }
    [[nodiscard]] const vk::Sampler& sampler() const { return _sampler; }

    [[nodiscard]] static TextureRef create(const TextureInfo& textureInfo);

private:
    vk::DeviceMemory _imageMemory;
    vk::Image _image;
    vk::ImageView _imageView;
    vk::Sampler _sampler;

    bool _generateMipmaps;
    uint32_t _mipLevels;
    uint32_t _width;
    uint32_t _height;

    void cleanup() const;
};

} // namespace chronicle