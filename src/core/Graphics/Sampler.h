// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

namespace chronicle::graphics
{

struct SamplerCreateInfo
{
    /// @brief Texture view name.
    std::string name{};

    AddressMode addressModeU{AddressMode::eClampToEdge};
    AddressMode addressModeV{AddressMode::eClampToEdge};
    AddressMode addressModeW{AddressMode::eClampToEdge};
    FilterMode magFilter{FilterMode::eNearest};
    FilterMode minFilter{FilterMode::eNearest};
    MipmapFilterMode mipmapFilter{MipmapFilterMode::eNearest};
    float lodMinClamp{0.0f};
    float lodMaxClamp{32.0f};
    std::optional<CompareFunction> compare{};
    uint32_t maxAnisotropy{1};
};

class Sampler
{
  public:
    virtual ~Sampler() = default;

    virtual std::string_view name() const = 0;
};

} // namespace chronicle::graphics
