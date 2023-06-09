// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

namespace chronicle::graphics
{

struct TextureViewCreateInfo
{
    /// @brief Texture view name.
    std::string name{};

    TextureFormat format{};
    TextureViewDimension dimension{TextureViewDimension::e2D};
    TextureAspect aspect{};
    uint32_t baseMipLevel{};
    uint32_t mipLevelCount{};
    uint32_t baseArrayLayer{};
    uint32_t arrayLayerCount{1};
};

class TextureView
{
  public:
    virtual ~TextureView() = default;

    virtual std::string_view name() const = 0;
    virtual TextureFormat format() const = 0;
    virtual TextureViewDimension dimension() const = 0;
    virtual TextureAspect aspect() const = 0;
    virtual uint32_t baseMipLevel() const = 0;
    virtual uint32_t mipLevelCount() const = 0;
    virtual uint32_t baseArrayLayer() const = 0;
    virtual uint32_t arrayLayerCount() const = 0;
};

} // namespace chronicle::graphics
