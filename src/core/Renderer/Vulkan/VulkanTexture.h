// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseTexture.h"

namespace chronicle::internal::vulkan {

/// @brief Vulkan implementation for @ref BaseTexture
class VulkanTexture : public BaseTexture<VulkanTexture>, private NonCopyable<VulkanTexture> {
protected:
    /// @brief Construct the sampled texture.
    /// @param textureInfo Informations used to create the texture.
    /// @param name Texture name.
    explicit VulkanTexture(const SampledTextureInfo& textureInfo, const std::string& name);

    /// @brief Construct the sampled texture.
    /// @param textureInfo Informations used to create the texture.
    /// @param name Texture name.
    explicit VulkanTexture(const ColorTextureInfo& textureInfo, const std::string& name);

    /// @brief Construct the sampled texture.
    /// @param textureInfo Informations used to create the texture.
    /// @param name Texture name.
    explicit VulkanTexture(const DepthTextureInfo& textureInfo, const std::string& name);

    /// @brief Construct the texture from an existing image (swapchain image).
    /// @param image Vulkan image.
    /// @param format Vulkan format.
    /// @param width Vulkan image width.
    /// @param height Vulkan image height.
    /// @param name Texture name.
    explicit VulkanTexture(
        const vk::Image& image, vk::Format format, uint32_t width, uint32_t height, const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanTexture();

    /// @brief @see BaseTexture#width
    [[nodiscard]] uint32_t width() const { return _width; }

    /// @brief @see BaseTexture#height
    [[nodiscard]] uint32_t height() const { return _height; }

    /// @brief @see BaseTexture#textureId
    [[nodiscard]] TextureId textureId() const { return _imageView; }

    /// @brief @see BaseTexture#samplerId
    [[nodiscard]] SamplerId samplerId() const { return _sampler; }

    /// @brief @see BaseTexture#createSampled
    [[nodiscard]] static TextureRef createSampled(const SampledTextureInfo& textureInfo, const std::string& name);

    /// @brief @see BaseTexture#createColor
    [[nodiscard]] static TextureRef createColor(const ColorTextureInfo& textureInfo, const std::string& name);

    /// @brief @see BaseTexture#createDepth
    [[nodiscard]] static TextureRef createDepth(const DepthTextureInfo& textureInfo, const std::string& name);

    /// @brief Factory for create a texture from an existing image (swapchain image).
    /// @param image Vulkan image.
    /// @param format Vulkan format.
    /// @param width Vulkan image width.
    /// @param height Vulkan image height.
    /// @param name Texture name.
    /// @return The texture.
    [[nodiscard]] static TextureRef createSwapchain(
        const vk::Image& image, vk::Format format, uint32_t width, uint32_t height, const std::string& name);

private:
    std::string _name {}; ///< Name.
    vk::DeviceMemory _imageMemory {}; ///< Device memory for the image.
    vk::Image _image {}; ///< Image.
    vk::ImageView _imageView {}; ///< Image view.
    vk::Sampler _sampler {}; ///< Image sampler.

    TextureType _type { TextureType::sampled }; ///< Texture type.
    Format _format { Format::undefined };
    bool _generateMipmaps { false }; ///< Generate mipmaps required.
    uint32_t _mipLevels {}; ///< Image miplevels.
    uint32_t _width {}; ///< Image width.
    uint32_t _height {}; ///< Image height.
};

} // namespace chronicle