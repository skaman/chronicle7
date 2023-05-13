// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/TextureI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref TextureI
class VulkanTexture : public TextureI<VulkanTexture>, private NonCopyable<VulkanTexture> {
protected:
    /// @brief Construct the sampled texture.
    /// @param textureInfo Informations used to create the texture.
    explicit VulkanTexture(const SampledTextureInfo& textureInfo);

    /// @brief Construct the sampled texture.
    /// @param textureInfo Informations used to create the texture.
    explicit VulkanTexture(const ColorTextureInfo& textureInfo);

    /// @brief Construct the sampled texture.
    /// @param textureInfo Informations used to create the texture.
    explicit VulkanTexture(const DepthTextureInfo& textureInfo);

    /// @brief Construct the texture from an existing image (swapchain image).
    /// @param image Vulkan image.
    /// @param format Vulkan format.
    /// @param width Vulkan image width.
    /// @param height Vulkan image height.
    explicit VulkanTexture(const vk::Image& image, vk::Format format, uint32_t width, uint32_t height);

public:
    /// @brief Destructor.
    ~VulkanTexture();

    /// @brief @see TextureI#width
    [[nodiscard]] uint32_t width() const { return _width; }

    /// @brief @see TextureI#height
    [[nodiscard]] uint32_t height() const { return _height; }

    /// @brief @see TextureI#textureId
    [[nodiscard]] TextureId textureId() const { return static_cast<TextureId>(&_imageView); }

    /// @brief @see TextureI#samplerId
    [[nodiscard]] SamplerId samplerId() const { return static_cast<SamplerId>(&_sampler); }

    /// @brief @see TextureI#createSampled
    [[nodiscard]] static TextureRef createSampled(const SampledTextureInfo& textureInfo);

    /// @brief @see TextureI#createColor
    [[nodiscard]] static TextureRef createColor(const ColorTextureInfo& textureInfo);

    /// @brief @see TextureI#createDepth
    [[nodiscard]] static TextureRef createDepth(const DepthTextureInfo& textureInfo);

    /// @brief Factory for create a texture from an existing image (swapchain image).
    /// @param image Vulkan image.
    /// @param format Vulkan format.
    /// @param width Vulkan image width.
    /// @param height Vulkan image height.
    /// @return The texture.
    [[nodiscard]] static TextureRef createSwapchain(
        const vk::Image& image, vk::Format format, uint32_t width, uint32_t height);

private:
    vk::DeviceMemory _imageMemory = nullptr; ///< Device memory for the image.
    vk::Image _image = nullptr; ///< Image.
    vk::ImageView _imageView = nullptr; ///< Image view.
    vk::Sampler _sampler = nullptr; ///< Image sampler.

    TextureType _type = TextureType::sampled; ///< Texture type.
    Format _format = Format::undefined;
    bool _generateMipmaps = false; ///< Generate mipmaps required.
    uint32_t _mipLevels = 0; ///< Image miplevels.
    uint32_t _width = 0; ///< Image width.
    uint32_t _height = 0; ///< Image height.
};

} // namespace chronicle