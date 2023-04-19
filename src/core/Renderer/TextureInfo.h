// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

/// @brief Informations used to create a texture.
struct TextureInfo {
    /// @brief Enabled the mipmap generation for the texture.
    bool generateMipmaps = true;
};

} // namespace chronicle