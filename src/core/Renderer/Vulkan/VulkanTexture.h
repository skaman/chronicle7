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
    explicit VulkanTexture() = default;

public:
    /// @brief Destructor.
    ~VulkanTexture();

    /// @brief @see TextureI#set
    void set(void* src, size_t size, uint32_t width, uint32_t height);

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
    vk::DeviceMemory _imageMemory; ///< Device memory for the image.
    vk::Image _image; ///< Image.
    vk::ImageView _imageView; ///< Image view.
    vk::Sampler _sampler; ///< Image sampler.

    bool _generateMipmaps; ///< Generate mipmaps required.
    uint32_t _mipLevels; ///< Image miplevels.
    uint32_t _width; ///< Image width.
    uint32_t _height; ///< Image height.

    /// @brief cleanup resources.
    void cleanup() const;
};

} // namespace chronicle