// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Informations used to create a sampled texture.
struct SampledTextureInfo {
    /// @brief Enabled the mipmap generation for the texture.
    bool generateMipmaps = true;

    /// @brief image data used to fill the texture.
    std::vector<uint8_t> data = {};

    /// @brief Texture width.
    uint32_t width = 0;

    /// @brief Texture height.
    uint32_t height = 0;
};

/// @brief Informations used to create a sampled texture.
struct ColorTextureInfo {
    /// @brief Texture width.
    uint32_t width = 0;

    /// @brief Texture height.
    uint32_t height = 0;

    /// @brief Surface format.
    Format format = Format::undefined;

    /// @brief MSAA sample count.
    MSAA msaa = MSAA::sampleCount1;

    /// @brief The texture will be used as an input attachment.
    bool isInputAttachment = false;

    /// @brief Generate mipmaps for the texture.
    bool generateMipmaps = false;
};

/// @brief Informations used to create a sampled texture.
struct DepthTextureInfo {
    /// @brief Texture width.
    uint32_t width = 0;

    /// @brief Texture height.
    uint32_t height = 0;

    /// @brief Depth format.
    Format format = Format::undefined;

    /// @brief MSAA sample count.
    MSAA msaa = MSAA::sampleCount1;
};

} // namespace chronicle