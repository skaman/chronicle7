// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/TextureI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref TextureI
class VulkanTexture : public TextureI<VulkanTexture>, private NonCopyable<VulkanTexture> {
protected:
    /// @brief Default constructor.
    explicit VulkanTexture(const TextureInfo& textureInfo);

public:
    /// @brief Destructor.
    ~VulkanTexture();

    /// @brief @see TextureI#width
    [[nodiscard]] uint32_t width() const { return _width; }

    /// @brief @see TextureI#height
    [[nodiscard]] uint32_t height() const { return _height; }

    /// @brief Get the image.
    /// @return Image.
    [[nodiscard]] const vk::Image& image() const { return _image; }

    /// @brief Get the image view.
    /// @return Image view.
    [[nodiscard]] const vk::ImageView& imageView() const { return _imageView; }

    /// @brief Get the image sampler.
    /// @return Image sampler.
    [[nodiscard]] const vk::Sampler& sampler() const { return _sampler; }

    /// @brief @see TextureI#create
    [[nodiscard]] static TextureRef create(const TextureInfo& textureInfo);

private:
    vk::DeviceMemory _imageMemory = nullptr; ///< Device memory for the image.
    vk::Image _image = nullptr; ///< Image.
    vk::ImageView _imageView = nullptr; ///< Image view.
    vk::Sampler _sampler = nullptr; ///< Image sampler.

    bool _generateMipmaps = false; ///< Generate mipmaps required.
    uint32_t _mipLevels = 0; ///< Image miplevels.
    uint32_t _width = 0; ///< Image width.
    uint32_t _height = 0; ///< Image height.
};

} // namespace chronicle