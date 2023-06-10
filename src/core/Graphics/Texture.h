// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"
#include "TextureView.h"

namespace chronicle::graphics
{

/// @brief Error exception for invalid texture operations.
class TextureError : public GraphicsError
{
  public:
    explicit TextureError(const std::string &message) : GraphicsError(message.c_str())
    {
    }
};

/// @brief Description for texture creation.
struct TextureDescriptor
{
    /// @brief Texture name.
    std::string name{};

    /// @brief The width of the texture (required).
    uint32_t width{};

    /// @brief The height of the texture (required).
    uint32_t height{};

    /// @brief The depth of the texture.
    uint32_t depth{1};

    /// @brief The array layers of the texture.
    uint32_t arrayLayers{1};

    /// @brief The number of mip levels the texture will contain.
    uint32_t mipLevelCount{1};

    /// @brief The sample count of the texture.
    TextureSampleCount sampleCount{TextureSampleCount::e1};

    /// @brief Whether the texture is one-dimensional, an array of two-dimensional layers, or three-dimensional.
    TextureDimension dimension{TextureDimension::e2D};

    /// @brief The format of the texture (required).
    TextureFormat format{};

    /// @brief The allowed usages for the texture (required).
    TextureUsageFlags usage{};
};

/// @brief One texture consists of one or more texture subresources, each uniquely identified by a mipmap level and, for
///        2d textures only, array layer and aspect.
class Texture
{
  public:
    /// @brief Constructor.
    /// @param bufferDescriptor Buffer descriptor.
    explicit Texture(const TextureDescriptor &textureDescriptor) : _textureDescriptor(textureDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~Texture() = default;

    /// @brief Create a texture view.
    /// @param textureViewDescriptor Texture view description.
    /// @return Texture view.
    virtual [[nodiscard]] std::shared_ptr<TextureView> createTextureView(
        const TextureViewDescriptor &textureViewDescriptor) const = 0;

    /// @brief Returns the descriptor used to create the texture.
    /// @return Texture descriptor.
    const TextureDescriptor &descriptor() const
    {
        return _textureDescriptor;
    }

  private:
    TextureDescriptor _textureDescriptor; ///< Texture descriptor.
};

} // namespace chronicle::graphics
