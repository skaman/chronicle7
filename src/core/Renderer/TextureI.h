// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "TextureInfo.h"

namespace chronicle {

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

    /// @brief Factory for create a new sampled texture.
    /// @param textureInfo Informations used to create the texture.
    /// @return The texture.
    [[nodiscard]] static TextureRef createSampled(const SampledTextureInfo& textureInfo)
    {
        return T::createSampled(textureInfo);
    }

    /// @brief Factory for create a new color texture.
    /// @param textureInfo Informations used to create the texture.
    /// @return The texture.
    [[nodiscard]] static TextureRef createColor(const ColorTextureInfo& textureInfo)
    {
        return T::createColor(textureInfo);
    }

    /// @brief Factory for create a new depth texture.
    /// @param textureInfo Informations used to create the texture.
    /// @return The texture.
    [[nodiscard]] static TextureRef createDepth(const DepthTextureInfo& textureInfo)
    {
        return T::createDepth(textureInfo);
    }

private:
    TextureI() = default;
    friend T;
};

} // namespace chronicle