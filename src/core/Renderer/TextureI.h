// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "TextureInfo.h"

namespace chronicle {

using TextureId = const void*;
using SamplerId = const void*;

/// @brief Object used to handle a texture.
/// @tparam T Type with implementation.
template <class T> class TextureI {
public:
    /// @brief Get the texture handle ID
    /// @return Texture ID
    [[nodiscard]] TextureId textureId() const { return static_cast<const T*>(this)->textureId(); }

    /// @brief Get the sampler handle ID
    /// @return Sampler ID
    [[nodiscard]] SamplerId samplerId() const { return static_cast<const T*>(this)->samplerId(); }

    /// @brief Factory for create a new texture.
    /// @param textureInfo Informations used to create the texture.
    /// @return The texture.
    [[nodiscard]] static TextureRef create(const TextureInfo& textureInfo) { return T::create(textureInfo); }

private:
    TextureI() = default;
    friend T;
};

} // namespace chronicle