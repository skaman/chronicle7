// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

/// @brief Informations used to create a texture.
struct TextureInfo {
    /// @brief Enabled the mipmap generation for the texture.
    bool generateMipmaps = true;

    /// @brief image data used to fill the texture.
    std::vector<uint8_t> data = {};

    /// @brief Texture width.
    uint32_t width = 0;

    /// @brief Texture height.
    uint32_t height = 0;
};

} // namespace chronicle