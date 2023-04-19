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
    /// @brief Set the texture image data.
    /// @param src A pointer to the data memory location.
    /// @param size Size of the data.
    /// @param width Texture width.
    /// @param height Texture height.
    void set(void* src, size_t size, uint32_t width, uint32_t height)
    {
        static_cast<T*>(this)->set(src, size, width, height);
    }

    /// @brief Factory for create a new texture.
    /// @param textureInfo Informations used to create the texture.
    /// @return The texture.
    [[nodiscard]] static TextureRef create(const TextureInfo& textureInfo) { return T::create(textureInfo); }

private:
    TextureI() = default;
    friend T;
};

} // namespace chronicle