// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

namespace chronicle::graphics
{

/// @brief Error exception for invalid texture view operations.
class TextureViewError : public GraphicsError
{
  public:
    explicit TextureViewError(const std::string &message) : GraphicsError(message.c_str())
    {
    }
};

/// @brief Description for texture view creation.
struct TextureViewDescriptor
{
    /// @brief Texture view name.
    std::string name{};

    /// @brief The format of the texture view.
    TextureFormat format{};

    /// @brief The dimension to view the texture as.
    TextureViewDimension dimension{TextureViewDimension::e2D};

    /// @brief Which aspect(s) of the texture are accessible to the texture view.
    TextureAspect aspect{TextureAspect::eColor};

    /// @brief The first (most detailed) mipmap level accessible to the texture view.
    uint32_t baseMipLevel{0};

    /// @brief How many mipmap levels, starting with baseMipLevel, are accessible to the texture view.
    uint32_t mipLevelCount{0};

    /// @brief The index of the first array layer accessible to the texture view.
    uint32_t baseArrayLayer{0};

    /// @brief How many array layers, starting with TextureViewDescriptor::baseArrayLayer, are accessible to the texture
    ///        view.
    uint32_t arrayLayerCount{1};
};

/// @brief A TextureView is a view onto some subset of the texture subresources defined by a particular Texture.
class TextureView
{
  public:
    /// @brief Constructor.
    /// @param textureViewDescriptor Texture view descriptor.
    explicit TextureView(const TextureViewDescriptor &textureViewDescriptor)
        : _textureViewDescriptor(textureViewDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~TextureView() = default;

    /// @brief Returns the descriptor used to create the texture view.
    /// @return Texture view descriptor.
    const TextureViewDescriptor &descriptor() const
    {
        return _textureViewDescriptor;
    }

  private:
    TextureViewDescriptor _textureViewDescriptor; ///< Texture view descriptor.
};

} // namespace chronicle::graphics
