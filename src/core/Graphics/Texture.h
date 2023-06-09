// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

namespace chronicle::graphics
{

struct TextureCreateInfo
{
    /// @brief Texture name.
    std::string name{};

    uint32_t width{};
    uint32_t height{};
    uint32_t depth{1};
    uint32_t arrayLayers{1};
    uint32_t mipLevelCount{};
    TextureViewDimension dimension{TextureViewDimension::e2D};
    TextureFormat format{};
    TextureUsageFlags usage{};
    TextureSampleCount sampleCount{};
};

class Texture
{
  public:
    virtual ~Texture() = default;

    virtual std::string_view name() const = 0;
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;
    virtual uint32_t depth() const = 0;
    virtual uint32_t arrayLayers() const = 0;
    virtual uint32_t mipLevelCount() const = 0;
    virtual TextureViewDimension dimension() const = 0;
    virtual TextureFormat format() const = 0;
    virtual TextureUsageFlags usage() const = 0;
    virtual TextureSampleCount sampleCount() const = 0;
};

} // namespace chronicle::graphics
